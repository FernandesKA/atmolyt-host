/**
 * @file csv_logger.cpp
 * @author FernandesKA (fernandes.kir@yandex.ru)
 * @brief  Implementation of the csv_logger class
 * @version 0.1
 * @date 2025-12-23
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "app/csv_logger.h"
#include <iostream>

namespace app
{
    csv_logger::csv_logger(const std::string& filename)
    {
        file_.open(filename, std::ios::app);
        if (!file_.is_open()) {
            std::cerr << "Failed to open CSV log file: " << filename << std::endl;
            return;
        }
        
        file_.seekp(0, std::ios::end);
        if (file_.tellp() == 0) {
            file_ << "timestamp,co2_ppm,temperature_c,pressure_pa,humidity_rh\n";
        }

        worker_ = std::thread(&csv_logger::worker_thread, this);
    }

    csv_logger::~csv_logger()
    {
        stop_ = true;
        cv_.notify_one();
        
        if (worker_.joinable()) {
            worker_.join();
        }
        
        if (file_.is_open()) {
            file_.close();
        }
    }

    void csv_logger::log(double co2_ppm, double temp_c, double press_pa, double humidity_rh, const std::string& timestamp)
    {
        if (file_.is_open()) {
            file_ << timestamp << "," << co2_ppm << "," << temp_c << "," << press_pa << "," << humidity_rh << "\n";
            file_.flush();
        }
    }

    void csv_logger::log_async(double co2_ppm, double temp_c, double press_pa, double humidity_rh, const std::string& timestamp)
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            queue_.push({co2_ppm, temp_c, press_pa, humidity_rh, timestamp});
        }
        cv_.notify_one();
    }

    void csv_logger::worker_thread()
    {
        while (!stop_) {
            std::unique_lock<std::mutex> lock(mutex_);
            cv_.wait(lock, [this] { return !queue_.empty() || stop_; });

            while (!queue_.empty()) {
                LogEntry entry = queue_.front();
                queue_.pop();
                lock.unlock();
                
                write_entry(entry);
                
                lock.lock();
            }
        }
        
        while (!queue_.empty()) {
            write_entry(queue_.front());
            queue_.pop();
        }
    }

    void csv_logger::write_entry(const LogEntry& entry)
    {
        if (file_.is_open()) {
            file_ << entry.timestamp << "," << entry.co2_ppm << "," 
                  << entry.temp_c << "," << entry.press_pa << "," 
                  << entry.humidity_rh << "\n";
            file_.flush();
        }
    }
}

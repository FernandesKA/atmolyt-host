/**
 * @file csv_logger.h
 * @author FernandesKA (fernandes.kir@yandex.ru)
 * @brief  Declaration of the csv_logger class
 * @version 0.1
 * @date 2025-12-23
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#pragma once

#include <fstream>
#include <string>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>

namespace app
{
    struct LogEntry {
        double co2_ppm;
        double temp_c;
        double press_pa;
        double humidity_rh;
        std::string timestamp;
    };

    class csv_logger
    {
    public:
        explicit csv_logger(const std::string& filename);
        ~csv_logger();

        void log(double co2_ppm, double temp_c, double press_pa, double humidity_rh, const std::string& timestamp);
        void log_async(double co2_ppm, double temp_c, double press_pa, double humidity_rh, const std::string& timestamp);

    private:
        void worker_thread();
        void write_entry(const LogEntry& entry);

        std::ofstream file_;
        std::queue<LogEntry> queue_;
        std::mutex mutex_;
        std::condition_variable cv_;
        std::thread worker_;
        std::atomic<bool> stop_{false};
    };
}

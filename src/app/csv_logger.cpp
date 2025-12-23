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
        
        // Write header if file is empty
        file_.seekp(0, std::ios::end);
        if (file_.tellp() == 0) {
            file_ << "timestamp,co2_ppm,temperature_c,pressure_pa,humidity_rh\n";
        }
    }

    csv_logger::~csv_logger()
    {
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
}

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

namespace app
{
    class csv_logger
    {
    public:
        explicit csv_logger(const std::string& filename);
        ~csv_logger();

        void log(double co2_ppm, double temp_c, double press_pa, double humidity_rh, const std::string& timestamp);

    private:
        std::ofstream file_;
    };
}

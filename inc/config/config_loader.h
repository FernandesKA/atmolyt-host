/**
 * @file config_loader.h
 * @author FernandesKA (fernandes.kir@yandex.ru)
 * @brief  Configuration loader for the application
 * @version 0.1
 * @date 2025-12-23
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace config {

struct PeripheralSpec {
    std::string connection; // "i2c" or "spi" or "mock"
    std::string type;       // "bme280", "bmp280", etc
    std::string device;     // e.g. "/dev/i2c-2" for i2c
    uint8_t address = 0;    // I2C address
};

struct AppConfig {
    std::vector<PeripheralSpec> peripherals;
    std::string log_path = "atmolyt_data.csv";
};

// Load config from file (JSON). Returns true on success and populates out
bool load_config(const std::string &path, AppConfig &out);

} // namespace config

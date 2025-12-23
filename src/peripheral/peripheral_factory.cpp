/**
 * @file peripheral_factory.cpp
 * @author FernandesKA (fernandes.kir@yandex.ru)
 * @brief  Peripheral factory implementation
 * @version 0.1
 * @date 2025-12-06
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "peripheral_factory.h"
#if TARGET_HOST
#include "peripheral/mock_environmental.h"
#else
#include "peripheral/bme280.h"
#include "peripheral/scd41.h"
#include "peripheral/ssd1306.h"
#include "peripheral/ds3231.h"
#endif
#include <stdexcept>
#include <algorithm>

namespace peripherals {

const std::map<std::string, PeripheralType> peripheral_factory::type_map_ = {
    {"bmp280", PeripheralType::BMP280},
    {"bme280", PeripheralType::BME280},
    {"mpu6050", PeripheralType::MPU6050},
    {"dht22", PeripheralType::DHT22},
    {"sgp41", PeripheralType::SGP41},
    {"scd41", PeripheralType::SCD41},
    {"ssd1306", PeripheralType::SSD1306},
    {"ds3231", PeripheralType::DS3231}
};

const std::map<PeripheralType, std::string> peripheral_factory::reverse_type_map_ = {
    {PeripheralType::BMP280, "bmp280"},
    {PeripheralType::BME280, "bme280"},
    {PeripheralType::MPU6050, "mpu6050"},
    {PeripheralType::DHT22, "dht22"},
    {PeripheralType::SGP41, "sgp41"},
    {PeripheralType::SCD41, "scd41"},
    {PeripheralType::SSD1306, "ssd1306"},
    {PeripheralType::DS3231, "ds3231"}
};

const std::map<PeripheralType, uint8_t> peripheral_factory::default_addresses_ = {
    {PeripheralType::BMP280, 0x76},
    {PeripheralType::BME280, 0x76},
    {PeripheralType::MPU6050, 0x68},
    {PeripheralType::DHT22, 0x00},
    {PeripheralType::SGP41, 0x59},
    {PeripheralType::SCD41, 0x62},
    {PeripheralType::SSD1306, 0x3C},
    {PeripheralType::DS3231, 0x68}
};

std::unique_ptr<environmental_sensor_iface>
peripheral_factory::create_environmental_sensor(
    PeripheralType type,
    connections::addressable_connection_iface<uint8_t>* conn,
    uint8_t address) {
    #if TARGET_HOST
    return std::make_unique<mock_environmental>(conn, address);
    #else
    switch (type) {
        case PeripheralType::BME280:
            return std::make_unique<bme280>(conn, address);

        case PeripheralType::BMP280:
            // BMP280 is not implemented here yet; fallback to BME280-compatible driver if available
            return std::make_unique<bme280>(conn, address);

        default:
            throw std::runtime_error("Unsupported environmental sensor type");
    }
    #endif
}

std::unique_ptr<imu_iface>
peripheral_factory::create_imu(
    PeripheralType type,
    connections::addressable_connection_iface<uint8_t>* conn,
    uint8_t address) {
    #if TARGET_HOST
    return nullptr;
    #else
    switch (type) {
        default:
            throw std::runtime_error("Unsupported IMU type");
    }
    #endif
}

std::unique_ptr<gas_sensor_iface>
peripheral_factory::create_gas_sensor(
    PeripheralType type,
    connections::addressable_connection_iface<uint8_t>* conn,
    uint8_t address) {
    #if TARGET_HOST
    return nullptr;
    #else
    switch (type) {
        case PeripheralType::SCD41:
            return std::make_unique<scd41>(conn, address);

        default:
            throw std::runtime_error("Unsupported gas sensor type");
    }
    #endif
}

std::unique_ptr<display_iface>
peripheral_factory::create_display(
    PeripheralType type,
    connections::addressable_connection_iface<uint8_t>* conn,
    uint8_t address) {
    #if TARGET_HOST
    return nullptr;
    #else
    switch (type) {
        case PeripheralType::SSD1306:
            return std::make_unique<ssd1306>(conn, address);

        default:
            throw std::runtime_error("Unsupported display type");
    }
    #endif
}

std::unique_ptr<rtc_iface>
peripheral_factory::create_rtc(
    PeripheralType type,
    connections::addressable_connection_iface<uint8_t>* conn,
    uint8_t address) {
    #if TARGET_HOST
    return nullptr;
    #else
    switch (type) {
        case PeripheralType::DS3231:
            return std::make_unique<ds3231>(conn, address);

        default:
            throw std::runtime_error("Unsupported RTC type");
    }
    #endif
}

PeripheralType peripheral_factory::string_to_type(const std::string& type_str) {
    std::string lower_str = type_str;
    std::transform(lower_str.begin(), lower_str.end(), lower_str.begin(),
                   [](unsigned char c){ return std::tolower(c); });
    
    auto it = type_map_.find(lower_str);
    if (it != type_map_.end()) {
        return it->second;
    }
    
    return PeripheralType::Unknown;
}

std::string peripheral_factory::type_to_string(PeripheralType type) {
    auto it = reverse_type_map_.find(type);
    if (it != reverse_type_map_.end()) {
        return it->second;
    }
    
    return "unknown";
}

uint8_t peripheral_factory::get_default_address(PeripheralType type) {
    auto it = default_addresses_.find(type);
    if (it != default_addresses_.end()) {
        return it->second;
    }
    
    return 0x00;
}

} // namespace peripherals

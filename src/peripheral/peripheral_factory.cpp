/**
 * @file peripheral_factory.cpp
 * @author FernandesKA (fernandes.kir@yandex.ru)
 * @brief 
 * @version 0.1
 * @date 2025-12-06
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "peripheral_factory.h"
#ifdef TARGET_HOST
#include "peripheral/mock_environmental.h"
#endif
#include <stdexcept>
#include <algorithm>

namespace peripherals {

const std::map<std::string, PeripheralType> peripheral_factory::type_map_ = {
    {"bmp280", PeripheralType::BMP280},
    {"mpu6050", PeripheralType::MPU6050},
    {"dht22", PeripheralType::DHT22},
    {"sgp41", PeripheralType::SGP41}
};

const std::map<PeripheralType, std::string> peripheral_factory::reverse_type_map_ = {
    {PeripheralType::BMP280, "bmp280"},
    {PeripheralType::MPU6050, "mpu6050"},
    {PeripheralType::DHT22, "dht22"},
    {PeripheralType::SGP41, "sgp41"}
};

const std::map<PeripheralType, uint8_t> peripheral_factory::default_addresses_ = {
    {PeripheralType::BMP280, 0x76},
    {PeripheralType::MPU6050, 0x68},
    {PeripheralType::DHT22, 0x00},
    {PeripheralType::SGP41, 0x59}
};

std::unique_ptr<environmental_sensor_iface>
peripheral_factory::create_environmental_sensor(
    PeripheralType type,
    connections::addressable_connection_iface<uint8_t>* conn,
    uint8_t address) {
    #ifdef TARGET_HOST
    return std::make_unique<mock_environmental>(conn, address);
    #else
    switch (type) {
        case PeripheralType::BMP280:
            return std::make_unique<bmp280>(conn, address);

        case PeripheralType::DHT22:
            return std::make_unique<dht22>(conn, address);

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
    #ifdef TARGET_HOST
    return nullptr;
    #else
    switch (type) {
        case PeripheralType::MPU6050:
            return std::make_unique<mpu6050>(conn, address);

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
    #ifdef TARGET_HOST
    return nullptr;
    #else
    switch (type) {
        case PeripheralType::SGP41:
            return std::make_unique<sgp41>(conn, address);

        default:
            throw std::runtime_error("Unsupported gas sensor type");
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

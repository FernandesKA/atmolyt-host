/**
 * @file periphery_factory.h
 * @author FernandesKA (fernandes.kir@yandex.ru)
 * @brief
 * @version 0.1
 * @date 2025-12-06
 *
 * @copyright Copyright (c) 2025
 *
 */

#pragma once

#include "peripheral_iface.h"
#include "connection_iface.h"
#include <memory>
#include <string>
#include <map>

namespace peripherals
{

    enum class PeripheralType
    {
        Unknown,
        BMP280,
        MPU6050,
        DHT22,
        SGP41
    };

    class peripheral_factory
    {
    public:
        peripheral_factory() = delete;

        static std::unique_ptr<environmental_sensor_iface>
        create_environmental_sensor(
            PeripheralType type,
            connections::addressable_connection_iface<uint8_t> *conn,
            uint8_t address);

        static std::unique_ptr<imu_iface>
        create_imu(
            PeripheralType type,
            connections::addressable_connection_iface<uint8_t> *conn,
            uint8_t address);

        static std::unique_ptr<gas_sensor_iface>
        create_gas_sensor(
            PeripheralType type,
            connections::addressable_connection_iface<uint8_t> *conn,
            uint8_t address);

        static PeripheralType string_to_type(const std::string &type_str);
        static std::string type_to_string(PeripheralType type);
        static uint8_t get_default_address(PeripheralType type);

    private:
        static const std::map<std::string, PeripheralType> type_map_;
        static const std::map<PeripheralType, std::string> reverse_type_map_;
        static const std::map<PeripheralType, uint8_t> default_addresses_;
    };

} // namespace peripherals

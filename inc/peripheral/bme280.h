/**
 * @file bme280.h
 * @author FernandesKA (fernandes.kir@yandex.ru)
 * @brief  BME280 environmental sensor interface
 * @version 0.1
 * @date 2025-12-06
 *
 * @copyright Copyright (c) 2025
 *
 */

#pragma once

#include "peripheral/peripheral_iface.h"
#include <array>

namespace peripherals {

class bme280 : public environmental_sensor_iface
{
public:
    bme280(connections::addressable_connection_iface<uint8_t> *conn, uint8_t address);
    ~bme280() override = default;

    Status initialize() override;
    void deinitialize() override;
    bool is_connected() override;
    Status reset() override;

    Status read_data(combined_env_data &data) override;

    Status read_temperature(temperature_data &data) override;
    Status read_humidity(humidity_data &data) override;
    Status read_pressure(pressure_data &data) override;

private:
    bool read_calibration();
    bool read_raw(int32_t &raw_t, int32_t &raw_p, int32_t &raw_h);

    // calibration params
    uint16_t dig_T1 = 0;
    int16_t dig_T2 = 0;
    int16_t dig_T3 = 0;

    uint16_t dig_P1 = 0;
    int16_t dig_P2 = 0;
    int16_t dig_P3 = 0;
    int16_t dig_P4 = 0;
    int16_t dig_P5 = 0;
    int16_t dig_P6 = 0;
    int16_t dig_P7 = 0;
    int16_t dig_P8 = 0;
    int16_t dig_P9 = 0;

    uint8_t dig_H1 = 0;
    int16_t dig_H2 = 0;
    uint8_t dig_H3 = 0;
    int16_t dig_H4 = 0;
    int16_t dig_H5 = 0;
    int8_t dig_H6 = 0;

    int32_t t_fine = 0;
};

} // namespace peripherals

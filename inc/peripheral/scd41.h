/**
 * @file scd41.h
 * @author FernandesKA (fernandes.kir@yandex.ru)
 * @brief  SCD41 CO2 sensor interface
 * @version 0.1
 * @date 2025-12-19
 *
 * @copyright Copyright (c) 2025
 *
 */

#pragma once

#include "peripheral/peripheral_iface.h"

namespace peripherals {

class scd41 : public gas_sensor_iface
{
public:
    scd41(connections::addressable_connection_iface<uint8_t> *conn, uint8_t address);
    ~scd41() override = default;

    Status initialize() override;
    void deinitialize() override;
    bool is_connected() override;
    Status reset() override;

    Status read_data(gas_data &data) override;

    Status set_measurement_mode(uint8_t mode) override;
    Status read_co2(float &ppm) override;
    Status read_tvoc(float &ppb) override;

private:
    Status start_periodic_measurement();
    Status stop_periodic_measurement();
    Status get_data_ready_status(bool &ready);
    Status read_measurement(uint16_t &co2_raw, uint16_t &temperature_raw, uint16_t &humidity_raw);

    // CRC calculation for SCD41
    uint8_t crc8(const uint8_t *data, size_t len);
    bool check_crc(const uint8_t *data, size_t len, uint8_t crc);

    // Conversion functions
    float convert_co2(uint16_t raw);
    float convert_temperature(uint16_t raw);
    float convert_humidity(uint16_t raw);
};

} // namespace peripherals
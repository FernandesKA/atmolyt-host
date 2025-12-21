/**
 * @file scd41.cpp
 * @author FernandesKA (fernandes.kir@yandex.ru)
 * @brief  SCD41 CO2 sensor implementation
 * @version 0.1
 * @date 2025-12-19
 *
 * @copyright Copyright (c) 2025
 *
 */

#include "peripheral/scd41.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <cstring>

namespace peripherals {

scd41::scd41(connections::addressable_connection_iface<uint8_t> *conn, uint8_t address)
    : gas_sensor_iface(conn, address)
{
}

Status scd41::initialize()
{
    if (initialized_) {
        return Status::Success;
    }

    // Stop any ongoing measurement
    stop_periodic_measurement();

    // Start periodic measurement
    Status status = start_periodic_measurement();
    if (status != Status::Success) {
        return status;
    }

    initialized_ = true;
    return Status::Success;
}

void scd41::deinitialize()
{
    if (initialized_) {
        stop_periodic_measurement();
        initialized_ = false;
    }
}

bool scd41::is_connected()
{
    // Try to read serial number or something to check connection
    // For simplicity, assume connected if initialized
    return initialized_;
}

Status scd41::reset()
{
    // SCD41 doesn't have a reset command, stop and restart measurement
    stop_periodic_measurement();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    return start_periodic_measurement();
}

Status scd41::read_data(gas_data &data)
{
    if (!initialized_) {
        return Status::ErrorNotInitialized;
    }

    uint16_t co2_raw, temp_raw, hum_raw;
    Status status = read_measurement(co2_raw, temp_raw, hum_raw);
    if (status != Status::Success) {
        data.valid = false;
        return status;
    }

    data.co2_ppm = convert_co2(co2_raw);
    data.tvoc_ppb = 0.0f; // SCD41 doesn't measure TVOC
    data.valid = true;
    last_read_time_ = std::chrono::steady_clock::now();

    return Status::Success;
}

Status scd41::set_measurement_mode(uint8_t mode)
{
    // SCD41 has only periodic measurement mode
    // Mode 0: periodic, others not supported
    if (mode == 0) {
        return start_periodic_measurement();
    }
    return Status::ErrorInvalidData;
}

Status scd41::read_co2(float &ppm)
{
    gas_data data;
    Status status = read_data(data);
    if (status == Status::Success) {
        ppm = data.co2_ppm;
    }
    return status;
}

Status scd41::read_tvoc(float &ppb)
{
    // SCD41 doesn't measure TVOC
    ppb = 0.0f;
    return Status::ErrorInvalidData;
}

Status scd41::start_periodic_measurement()
{
    // Command: 0x21b1
    uint8_t cmd[2] = {0x21, 0xb1};
    auto status = connection_->write(device_address_, std::span(cmd, 2));
    return status == connections::Status::Success ? Status::Success : Status::ErrorCommunication;
}

Status scd41::stop_periodic_measurement()
{
    // Command: 0x3f86
    uint8_t cmd[2] = {0x3f, 0x86};
    auto status = connection_->write(device_address_, std::span(cmd, 2));
    return status == connections::Status::Success ? Status::Success : Status::ErrorCommunication;
}

Status scd41::get_data_ready_status(bool &ready)
{
    // Command: 0xe4b8
    uint8_t cmd[2] = {0xe4, 0xb8};
    uint8_t response[3];
    auto status = connection_->write_read(device_address_, std::span(cmd, 2), std::span(response, 3));
    if (status != connections::Status::Success) {
        return Status::ErrorCommunication;
    }

    // Check CRC
    if (!check_crc(response, 2, response[2])) {
        return Status::ErrorInvalidData;
    }

    uint16_t status_word = (response[0] << 8) | response[1];
    ready = (status_word & 0x07FF) != 0;
    return Status::Success;
}

Status scd41::read_measurement(uint16_t &co2_raw, uint16_t &temperature_raw, uint16_t &humidity_raw)
{
    // Wait until data is ready
    bool ready = false;
    for (int i = 0; i < 10; ++i) { // Timeout after ~5 seconds
        Status status = get_data_ready_status(ready);
        if (status != Status::Success) {
            return status;
        }
        if (ready) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    if (!ready) {
        return Status::ErrorTimeout;
    }

    // Command: 0xec05
    uint8_t cmd[2] = {0xec, 0x05};
    uint8_t response[9];
    auto status = connection_->write_read(device_address_, std::span(cmd, 2), std::span(response, 9));
    if (status != connections::Status::Success) {
        return Status::ErrorCommunication;
    }

    // Check CRCs
    if (!check_crc(response, 2, response[2]) ||
        !check_crc(response + 3, 2, response[5]) ||
        !check_crc(response + 6, 2, response[8])) {
        return Status::ErrorInvalidData;
    }

    co2_raw = (response[0] << 8) | response[1];
    temperature_raw = (response[3] << 8) | response[4];
    humidity_raw = (response[6] << 8) | response[7];

    return Status::Success;
}

uint8_t scd41::crc8(const uint8_t *data, size_t len)
{
    uint8_t crc = 0xFF;
    for (size_t i = 0; i < len; ++i) {
        crc ^= data[i];
        for (uint8_t bit = 8; bit > 0; --bit) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ 0x31;
            } else {
                crc = (crc << 1);
            }
        }
    }
    return crc;
}

bool scd41::check_crc(const uint8_t *data, size_t len, uint8_t crc)
{
    return crc8(data, len) == crc;
}

float scd41::convert_co2(uint16_t raw)
{
    return static_cast<float>(raw);
}

float scd41::convert_temperature(uint16_t raw)
{
    return -45.0f + 175.0f * static_cast<float>(raw) / 65535.0f;
}

float scd41::convert_humidity(uint16_t raw)
{
    return 100.0f * static_cast<float>(raw) / 65535.0f;
}

} // namespace peripherals
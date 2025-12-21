/**
 * @file ds3231.cpp
 * @author FernandesKA (fernandes.kir@yandex.ru)
 * @brief  DS3231 RTC implementation
 * @version 0.1
 * @date 2025-12-20
 *
 * @copyright Copyright (c) 2025
 *
 */

#include "peripheral/ds3231.h"
#include <iostream>
#include <thread>
#include <chrono>

namespace peripherals {

ds3231::ds3231(connections::addressable_connection_iface<uint8_t> *conn, uint8_t address)
    : rtc_iface(conn, address)
{
}

Status ds3231::initialize()
{
    if (initialized_) {
        return Status::Success;
    }

    // Check if device is present
    uint8_t reg;
    if (read_register(0x00, reg) != Status::Success) {
        return Status::ErrorCommunication;
    }

    initialized_ = true;
    return Status::Success;
}

void ds3231::deinitialize()
{
    initialized_ = false;
}

bool ds3231::is_connected()
{
    uint8_t reg;
    return read_register(0x00, reg) == Status::Success;
}

Status ds3231::reset()
{
    deinitialize();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    return initialize();
}

Status ds3231::read_data(time_data &data)
{
    uint8_t buffer[7];
    if (read_registers(0x00, std::span(buffer, 7)) != Status::Success) {
        return Status::ErrorCommunication;
    }

    // Convert BCD to decimal
    uint8_t sec, min, hour, day, date, month, year;
    if (bcd_to_dec(buffer[0], sec) != Status::Success ||
        bcd_to_dec(buffer[1], min) != Status::Success ||
        bcd_to_dec(buffer[2], hour) != Status::Success ||
        bcd_to_dec(buffer[3], day) != Status::Success ||
        bcd_to_dec(buffer[4], date) != Status::Success ||
        bcd_to_dec(buffer[5], month) != Status::Success ||
        bcd_to_dec(buffer[6], year) != Status::Success) {
        return Status::ErrorInvalidData;
    }

    data.second = sec;
    data.minute = min;
    data.hour = hour;
    data.day = date;
    data.month = month;
    data.year = 2000 + year; // Assuming 21st century
    data.valid = true;

    return Status::Success;
}

Status ds3231::set_time(const time_data &time)
{
    uint8_t buffer[7];

    // Convert decimal to BCD
    if (dec_to_bcd(time.second, buffer[0]) != Status::Success ||
        dec_to_bcd(time.minute, buffer[1]) != Status::Success ||
        dec_to_bcd(time.hour, buffer[2]) != Status::Success ||
        dec_to_bcd(time.day, buffer[3]) != Status::Success ||
        dec_to_bcd(time.day, buffer[4]) != Status::Success || // Day of week, using day for simplicity
        dec_to_bcd(time.month, buffer[5]) != Status::Success ||
        dec_to_bcd(time.year % 100, buffer[6]) != Status::Success) {
        return Status::ErrorInvalidData;
    }

    // Write to registers
    for (size_t i = 0; i < 7; ++i) {
        if (write_register(0x00 + i, buffer[i]) != Status::Success) {
            return Status::ErrorCommunication;
        }
    }

    return Status::Success;
}

Status ds3231::bcd_to_dec(uint8_t bcd, uint8_t &dec)
{
    dec = ((bcd >> 4) * 10) + (bcd & 0x0F);
    return Status::Success;
}

Status ds3231::dec_to_bcd(uint8_t dec, uint8_t &bcd)
{
    bcd = ((dec / 10) << 4) | (dec % 10);
    return Status::Success;
}

} // namespace peripherals
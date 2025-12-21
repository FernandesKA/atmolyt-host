/**
 * @file ds3231.h
 * @author FernandesKA (fernandes.kir@yandex.ru)
 * @brief  DS3231 RTC interface
 * @version 0.1
 * @date 2025-12-20
 *
 * @copyright Copyright (c) 2025
 *
 */

#pragma once

#include "peripheral/peripheral_iface.h"

namespace peripherals {

class ds3231 : public rtc_iface
{
public:
    ds3231(connections::addressable_connection_iface<uint8_t> *conn, uint8_t address);
    ~ds3231() override = default;

    Status initialize() override;
    void deinitialize() override;
    bool is_connected() override;
    Status reset() override;

    Status read_data(time_data &data) override;
    Status set_time(const time_data &time) override;

private:
    Status bcd_to_dec(uint8_t bcd, uint8_t &dec);
    Status dec_to_bcd(uint8_t dec, uint8_t &bcd);
};

} // namespace peripherals
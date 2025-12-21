/**
 * @file i2c_connection.h
 * @author FernandesKA (fernandes.kir@yandex.ru)
 * @brief  I2C connection implementation
 * @version 0.1
 * @date 2025-12-06
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#pragma once

#include "connection_iface.h"
#include <string>

namespace connections {

class i2c_connection : public addressable_connection_iface<uint8_t> {
public:
    explicit i2c_connection(std::string_view device_path);
    ~i2c_connection() override;
    
    Status initialize() override;
    void deinitialize() override;
    bool is_ready() const override;
    
    Status read(std::span<uint8_t> buffer) override;
    Status write(std::span<const uint8_t> data) override;
    
    Status read(uint8_t device_addr, std::span<uint8_t> buffer) override;
    Status write(uint8_t device_addr, std::span<const uint8_t> data) override;
    
    Status read_register(uint8_t device_addr, uint8_t reg_addr, 
                        std::span<uint8_t> buffer) override;
    Status write_register(uint8_t device_addr, uint8_t reg_addr,
                         std::span<const uint8_t> data) override;
    
    Status write_read(uint8_t device_addr, std::span<const uint8_t> write_data, std::span<uint8_t> read_buffer) override;
    
    Status reset() override;
    void flush() override;

private:
    int fd_;
};

} // namespace connections

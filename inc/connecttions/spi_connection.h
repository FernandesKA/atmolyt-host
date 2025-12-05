/**
 * @file spi_connection.h
 * @author FernandesKA (fernandes.kir@yandex.ru)
 * @brief 
 * @version 0.1
 * @date 2025-12-06
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#pragma once

#include "connection_iface.h"
#include <cstdint>

namespace connections {

struct spi_config {
    uint32_t speed_hz = 1000000;
    uint8_t mode = 0;
    uint8_t bits_per_word = 8;
};

class spi_connection : public addressable_connection_iface<uint8_t> {
public:
    explicit spi_connection(std::string_view device_path, 
                           const spi_config& config = {});
    ~spi_connection() override;
    
    Status initialize() override;
    void deinitialize() override;
    bool is_ready() const override;
    
    Status read(std::span<uint8_t> buffer) override;
    Status write(std::span<const uint8_t> data) override;
    
    Status read(uint8_t cs_pin, std::span<uint8_t> buffer) override;
    Status write(uint8_t cs_pin, std::span<const uint8_t> data) override;
    
    Status read_register(uint8_t cs_pin, uint8_t reg_addr, 
                        std::span<uint8_t> buffer) override;
    Status write_register(uint8_t cs_pin, uint8_t reg_addr,
                         std::span<const uint8_t> data) override;
    
    Status transfer(uint8_t cs_pin, std::span<const uint8_t> tx_data,
                   std::span<uint8_t> rx_data);
    
    Status reset() override;
    void flush() override;
    
    void set_config(const spi_config& config);

private:
    Status apply_config();
    
    int fd_;
    spi_config config_;
};

} // namespace connections

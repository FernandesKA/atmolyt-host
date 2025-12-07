/**
 * @file uart_connection.h
 * @author FernandesKA (fernandes.kir@yandex.ru)
 * @brief  UART connection implementation
 * @version 0.1
 * @date 2025-12-06
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#pragma once

#include "connection_iface.h"
#include <cstdint>
#include <termios.h>

namespace connections {

enum class uart_parity {
    none,
    even,
    odd
};

enum class uart_stop_bits {
    one,
    two
};

struct uart_config {
    uint32_t baudrate = 115200;
    uint8_t data_bits = 8;
    uart_parity parity = uart_parity::none;
    uart_stop_bits stop_bits = uart_stop_bits::one;
    bool hardware_flow_control = false;
};

class uart_connection : public connection_iface<uint8_t> {
public:
    explicit uart_connection(std::string_view device_path,
                            const uart_config& config = {});
    ~uart_connection() override;
    
    Status initialize() override;
    void deinitialize() override;
    bool is_ready() const override;
    
    Status read(std::span<uint8_t> buffer) override;
    Status write(std::span<const uint8_t> data) override;
    
    Status read_timeout(std::span<uint8_t> buffer, uint32_t timeout_ms) override;
    Status write_timeout(std::span<const uint8_t> data, uint32_t timeout_ms) override;
    
    Status reset() override;
    void flush() override;
    
    void set_config(const uart_config& config);
    size_t available() const;

private:
    Status apply_config();
    speed_t baudrate_to_speed(uint32_t baudrate) const;
    
    int fd_;
    uart_config config_;
};

} // namespace connections

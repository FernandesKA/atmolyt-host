/**
 * @file connection_iface.h
 * @author FernandesKA (fernandes.kir@yandex.ru)
 * @brief Base interface for hardware connections (I2C, SPI, UART, etc.)
 * @version 0.1
 * @date 2025-12-06
 * 
 * @copyright Copyright (c) 2025
 */

#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <functional>
#include <cstdint>
#include <optional>
#include <span>

namespace connections {

enum class Status {
    Success = 0,
    ErrorTimeout,
    ErrorNack,
    ErrorBusy,
    ErrorInvalidParam,
    ErrorNotInitialized,
    ErrorHardware
};

template<typename T = uint8_t>
class connection_iface {
public:
    explicit connection_iface(std::string_view config_path)
        : config_path_(config_path) {}
    
    virtual ~connection_iface() = default;

    connection_iface(const connection_iface&) = delete;
    connection_iface& operator=(const connection_iface&) = delete;
    connection_iface(connection_iface&&) noexcept = default;
    connection_iface& operator=(connection_iface&&) noexcept = default;

    using read_callback_t = std::function<void(std::span<const T> data, Status status)>;
    using write_callback_t = std::function<void(size_t bytes_written, Status status)>;

    virtual Status initialize() = 0;
    virtual void deinitialize() = 0;
    virtual bool is_ready() const = 0;

    virtual Status read(std::span<T> buffer) = 0;
    virtual Status write(std::span<const T> data) = 0;
    
    virtual Status read_timeout(std::span<T> buffer, uint32_t timeout_ms) {
        return read(buffer);
    }
    
    virtual Status write_timeout(std::span<const T> data, uint32_t timeout_ms) {
        return write(data);
    }

    virtual Status read_async(std::span<T> buffer, read_callback_t callback) {
        return Status::ErrorNotInitialized;
    }
    
    virtual Status write_async(std::span<const T> data, write_callback_t callback) {
        return Status::ErrorNotInitialized;
    }

    virtual Status reset() = 0;
    virtual void flush() = 0;

    std::string_view get_config_path() const { return config_path_; }

protected:
    std::string config_path_;
    bool initialized_ = false;
};

template<typename T = uint8_t>
class addressable_connection_iface : public connection_iface<T> {
public:
    using connection_iface<T>::connection_iface;
    
    virtual Status read(uint8_t device_addr, std::span<T> buffer) = 0;
    virtual Status write(uint8_t device_addr, std::span<const T> data) = 0;
    
    virtual Status read_register(uint8_t device_addr, uint8_t reg_addr, 
                                 std::span<T> buffer) = 0;
    
    virtual Status write_register(uint8_t device_addr, uint8_t reg_addr,
                                  std::span<const T> data) = 0;
};

} // namespace connections

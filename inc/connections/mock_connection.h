/**
 * @file mock_connection.h
 * @author FernandesKA (fernandes.kir@yandex.ru)
 * @brief  Mock connection implementation for testing
 * @version 0.1
 * @date 2025-12-07
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#pragma once

#include "connections/connection_iface.h"
#include <vector>

namespace connections
{

    class mock_addressable_connection : public addressable_connection_iface<uint8_t>
    {
    public:
        mock_addressable_connection(std::string_view cfg = "") : addressable_connection_iface<uint8_t>(cfg) {}
        virtual ~mock_addressable_connection() = default;

        Status initialize() override { initialized_ = true; return Status::Success; }
        void deinitialize() override { initialized_ = false; }
        bool is_ready() const override { return initialized_; }

        Status read(std::span<uint8_t> buffer) override {
            for (size_t i = 0; i < buffer.size(); ++i) buffer[i] = 0xFF;
            return Status::Success;
        }
        Status write(std::span<const uint8_t> data) override { (void)data; return Status::Success; }

        Status read(uint8_t device_addr, std::span<uint8_t> buffer) override {
            (void)device_addr;
            for (size_t i = 0; i < buffer.size(); ++i) buffer[i] = static_cast<uint8_t>(i & 0xFF);
            return Status::Success;
        }
        Status write(uint8_t device_addr, std::span<const uint8_t> data) override { (void)device_addr; (void)data; return Status::Success; }

        Status read_register(uint8_t device_addr, uint8_t reg_addr, std::span<uint8_t> buffer) override {
            (void)device_addr;
            // Return realistic ID for BME280/BMP280 when ID register requested
            if (reg_addr == 0xD0 && buffer.size() > 0) {
                // 0x60 = BME280, 0x58 = BMP280; return BME280 by default
                buffer[0] = 0x60;
                // fill remaining bytes with incrementing values
                for (size_t i = 1; i < buffer.size(); ++i) buffer[i] = static_cast<uint8_t>(reg_addr + i);
                return Status::Success;
            }
            for (size_t i = 0; i < buffer.size(); ++i) buffer[i] = static_cast<uint8_t>(reg_addr + i);
            return Status::Success;
        }

        Status write_register(uint8_t device_addr, uint8_t reg_addr, std::span<const uint8_t> data) override {
            (void)device_addr; (void)reg_addr; (void)data; return Status::Success;
        }

        Status reset() override { initialized_ = false; return Status::Success; }
        void flush() override {}
    };

} // namespace connections

/**
 * @file test_connection_mock.h
 * @author FernandesKA (fernandes.kir@yandex.ru)
 * @brief  Test double for connection interface
 * @version 0.1
 * @date 2025-12-23
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#pragma once

#include "connections/connection_iface.h"
#include <span>
#include <cstdint>
#include <map>

namespace connections {

// Test double for connection interface - records all calls and returns predefined responses
class test_connection_mock : public addressable_connection_iface<uint8_t>
{
public:
    test_connection_mock() : addressable_connection_iface<uint8_t>("test") {}

    Status initialize() override { initialized_ = true; return Status::Success; }
    void deinitialize() override { initialized_ = false; }
    bool is_ready() const override { return initialized_; }

    Status read(std::span<uint8_t> buffer) override {
        for (size_t i = 0; i < buffer.size(); ++i) buffer[i] = 0xAA;
        return Status::Success;
    }
    Status write(std::span<const uint8_t> data) override { (void)data; return Status::Success; }

    Status read(uint8_t device_addr, std::span<uint8_t> buffer) override {
        (void)device_addr;
        for (size_t i = 0; i < buffer.size(); ++i) buffer[i] = 0xBB;
        return Status::Success;
    }
    Status write(uint8_t device_addr, std::span<const uint8_t> data) override {
        (void)device_addr; (void)data;
        return Status::Success;
    }

    Status read_register(uint8_t device_addr, uint8_t reg_addr, std::span<uint8_t> buffer) override {
        (void)device_addr;
        // return mock calibration data for BME280 tests
        std::map<uint8_t, uint8_t> regs = {
            {0x88, 0x6A}, {0x89, 0x67}, // dig_T1
            {0xE1, 0x5C}, {0xE2, 0x00}, // dig_H2
            {0xD0, 0x60}, // chip ID (BME280)
        };

        for (size_t i = 0; i < buffer.size(); ++i) {
            auto it = regs.find(reg_addr + i);
            buffer[i] = (it != regs.end()) ? it->second : 0;
        }
        return Status::Success;
    }

    Status write_register(uint8_t device_addr, uint8_t reg_addr, std::span<const uint8_t> data) override {
        (void)device_addr; (void)reg_addr; (void)data;
        return Status::Success;
    }

    Status write_read(uint8_t device_addr, std::span<const uint8_t> write_data, std::span<uint8_t> read_buffer) override {
        (void)device_addr; (void)write_data;
        for (size_t i = 0; i < read_buffer.size(); ++i) read_buffer[i] = static_cast<uint8_t>(i & 0xFF);
        return Status::Success;
    }

    Status reset() override { return Status::Success; }
    void flush() override {}
};

} // namespace connections

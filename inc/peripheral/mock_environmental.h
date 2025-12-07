#pragma once

#include "peripheral/peripheral_iface.h"

namespace peripherals
{

    class mock_environmental : public environmental_sensor_iface
    {
    public:
        mock_environmental(connections::addressable_connection_iface<uint8_t> *conn, uint8_t address)
            : environmental_sensor_iface(conn, address) {}

        Status initialize() override { initialized_ = true; return Status::Success; }
        void deinitialize() override { initialized_ = false; }
        bool is_connected() override { return initialized_; }
        Status reset() override { initialized_ = true; return Status::Success; }

        Status read_data(combined_env_data &data) override { (void)data; return Status::Success; }

        Status read_temperature(temperature_data &data) override {
            data.celsius = 25.0f; data.valid = true; return Status::Success;
        }

        Status read_humidity(humidity_data &data) override {
            data.relative_humidity = 50.0f; data.valid = true; return Status::Success;
        }

        Status read_pressure(pressure_data &data) override {
            data.pascals = 101325.0f; data.valid = true; return Status::Success;
        }
    };

} // namespace peripherals

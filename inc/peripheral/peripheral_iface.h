/**
 * @file peripheral_iface.h
 * @author FernandesKA (fernandes.kir@yandex.ru)
 * @brief  Base interface for peripheral sensors
 * @version 0.1
 * @date 2025-12-06
 *
 * @copyright Copyright (c) 2025
 *
 */
#pragma once

#include "connections/connection_iface.h"
#include <cstdint>
#include <string_view>
#include <optional>
#include <chrono>
#include <variant>
#include <system_error>

namespace peripherals
{
    enum class Status
    {
        Success = 0,
        ErrorNotInitialized,
        ErrorCommunication,
        ErrorInvalidData,
        ErrorTimeout,
        ErrorCalibration,
        ErrorOutOfRange
    };

    // Modern error handling: Result<T> = variant<T, Status>
    template<typename T>
    class [[nodiscard]] Result {
    public:
        Result(T value) : data_(std::move(value)) {}
        Result(Status error) : data_(error) {}
        
        explicit operator bool() const noexcept { return std::holds_alternative<T>(data_); }
        bool has_value() const noexcept { return std::holds_alternative<T>(data_); }
        
        T& value() & { return std::get<T>(data_); }
        const T& value() const & { return std::get<T>(data_); }
        T&& value() && { return std::get<T>(std::move(data_)); }
        
        T value_or(T&& default_value) const & {
            return has_value() ? value() : std::move(default_value);
        }
        
        Status error() const { return std::get<Status>(data_); }
        
    private:
        std::variant<T, Status> data_;
    };

    // Specialization for void (no return value, just Status)
    template<>
    class [[nodiscard]] Result<void> {
    public:
        Result() : error_(Status::Success) {}
        Result(Status error) : error_(error) {}
        
        explicit operator bool() const noexcept { return error_ == Status::Success; }
        bool has_value() const noexcept { return error_ == Status::Success; }
        
        Status error() const { return error_; }
        
        // For compatibility with old code
        operator Status() const { return error_; }
        
    private:
        Status error_;
    };

    template <typename T>
    class peripheral_iface
    {
    public:
        peripheral_iface(connections::addressable_connection_iface<uint8_t> *conn,
                         uint8_t address)
            : connection_(conn), device_address_(address), initialized_(false) {}

        virtual ~peripheral_iface() = default;

        peripheral_iface(const peripheral_iface &) = delete;
        peripheral_iface &operator=(const peripheral_iface &) = delete;

        virtual Status initialize() = 0;
        virtual void deinitialize() = 0;
        virtual bool is_connected() = 0;
        virtual Status reset() = 0;

        virtual Status read_data(T &data) = 0;

        bool is_initialized() const { return initialized_; }
        uint8_t get_address() const { return device_address_; }

        auto get_last_read_time() const { return last_read_time_; }

    protected:
        connections::addressable_connection_iface<uint8_t> *connection_;
        uint8_t device_address_;
        bool initialized_;
        std::chrono::steady_clock::time_point last_read_time_;

        Status write_register(uint8_t reg, uint8_t value)
        {
            auto status = connection_->write_register(device_address_, reg,
                                                      std::span(&value, 1));
            return status == connections::Status::Success ? Status::Success : Status::ErrorCommunication;
        }

        Status read_register(uint8_t reg, uint8_t &value)
        {
            auto status = connection_->read_register(device_address_, reg,
                                                     std::span(&value, 1));
            return status == connections::Status::Success ? Status::Success : Status::ErrorCommunication;
        }

        Status read_registers(uint8_t reg, std::span<uint8_t> buffer)
        {
            auto status = connection_->read_register(device_address_, reg, buffer);
            return status == connections::Status::Success ? Status::Success : Status::ErrorCommunication;
        }
    };

    struct temperature_data
    {
        float celsius;
        bool valid;
    };

    struct humidity_data
    {
        float relative_humidity;
        bool valid;
    };

    struct pressure_data
    {
        float pascals;
        bool valid;
    };

    struct combined_env_data
    {
        temperature_data temperature;
        humidity_data humidity;
        pressure_data pressure;
    };

    struct gyro_data
    {
        float x_dps;
        float y_dps;
        float z_dps;
        bool valid;
    };

    struct accel_data
    {
        float x_g;
        float y_g;
        float z_g;
        bool valid;
    };

    struct imu_data
    {
        accel_data acceleration;
        gyro_data gyroscope;
    };

    struct gas_data
    {
        float co2_ppm;
        float tvoc_ppb;
        float temperature_c;
        float humidity_rh;
        bool valid;
    };
    struct time_data
    {
        int year;
        int month;
        int day;
        int hour;
        int minute;
        int second;
        bool valid;
    };
    class temperature_sensor_iface : public peripheral_iface<temperature_data>
    {
    public:
        using peripheral_iface::peripheral_iface;

        virtual Status set_resolution(uint8_t bits) = 0;
        virtual Status set_sampling_rate(uint16_t rate_hz) = 0;
    };

    class humidity_sensor_iface : public peripheral_iface<humidity_data>
    {
    public:
        using peripheral_iface::peripheral_iface;

        virtual Status set_resolution(uint8_t bits) = 0;
    };

    class pressure_sensor_iface : public peripheral_iface<pressure_data>
    {
    public:
        using peripheral_iface::peripheral_iface;

        virtual Status set_oversampling(uint8_t factor) = 0;
    };

    class environmental_sensor_iface : public peripheral_iface<combined_env_data>
    {
    public:
        using peripheral_iface::peripheral_iface;

        virtual Status read_temperature(temperature_data &data) = 0;
        virtual Status read_humidity(humidity_data &data) = 0;
        virtual Status read_pressure(pressure_data &data) = 0;
    };

    class gyroscope_iface : public peripheral_iface<gyro_data>
    {
    public:
        using peripheral_iface::peripheral_iface;

        virtual Status set_range(uint16_t dps) = 0;
        virtual Status set_data_rate(uint16_t rate_hz) = 0;
        virtual Status calibrate() = 0;
    };

    class accelerometer_iface : public peripheral_iface<accel_data>
    {
    public:
        using peripheral_iface::peripheral_iface;

        virtual Status set_range(uint8_t g_range) = 0;
        virtual Status set_data_rate(uint16_t rate_hz) = 0;
    };

    class imu_iface : public peripheral_iface<imu_data>
    {
    public:
        using peripheral_iface::peripheral_iface;

        virtual Status read_acceleration(accel_data &data) = 0;
        virtual Status read_gyroscope(gyro_data &data) = 0;
        virtual Status calibrate() = 0;
    };

    class gas_sensor_iface : public peripheral_iface<gas_data>
    {
    public:
        using peripheral_iface::peripheral_iface;

        virtual Status set_measurement_mode(uint8_t mode) = 0;
        virtual Status read_co2(float &ppm) = 0;
        virtual Status read_tvoc(float &ppb) = 0;
    };

    struct display_data
    {
        // Dummy struct for display
    };

    class display_iface : public peripheral_iface<display_data>
    {
    public:
        using peripheral_iface::peripheral_iface;

        virtual Status clear() = 0;
        virtual Status display_text(const std::string &text, uint8_t x = 0, uint8_t y = 0, uint8_t scale = 1, bool bold = false) = 0;
        virtual Status set_cursor(uint8_t x, uint8_t y) = 0;
        virtual void draw_pixel(int x, int y, int color) = 0;
        virtual void draw_line(int x0, int y0, int x1, int y1, int color) = 0;
    };

    class rtc_iface : public peripheral_iface<time_data>
    {
    public:
        using peripheral_iface::peripheral_iface;

        virtual Status set_time(const time_data &time) = 0;
    };

} // namespace peripherals
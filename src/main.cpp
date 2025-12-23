/**
 * @file main.cpp
 * @author FernandesKA (fernandes.kir@yandex.ru)
 * @brief  Main application entry point
 * @version 0.1
 * @date 2025-12-23
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "app/application.h"
#include "app/signal_handler.h"
#include "app/csv_logger.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <future>
#include <optional>

using app::signal_handler;

struct sensor_data {
    double co2_ppm = -1;
    double temp_c = -999;
    double humidity_rh = -1;
    double press_pa = -1;
    bool valid = false;
};

std::optional<sensor_data> read_gas_sensors_async(
    const std::vector<std::unique_ptr<peripherals::gas_sensor_iface>>& sensors)
{
    for (auto& sensor : sensors) {
        peripherals::gas_data data;
        auto result = sensor->read_data(data);
        if (result == peripherals::Status::Success) {
            return sensor_data{
                data.co2_ppm,
                data.temperature_c,
                data.humidity_rh,
                -1,
                true
            };
        }
    }
    return std::nullopt;
}

std::optional<sensor_data> read_env_sensors_async(
    const std::vector<std::unique_ptr<peripherals::environmental_sensor_iface>>& sensors)
{
    for (auto& sensor : sensors) {
        peripherals::combined_env_data data;
        auto result = sensor->read_data(data);
        if (result == peripherals::Status::Success) {
            return sensor_data{
                -1,
                data.temperature.celsius,
                -1,
                data.pressure.pascals,
                true
            };
        }
    }
    return std::nullopt;
}

int main(int argc, char **argv)
{
    signal_handler::install();

    signal_handler::set_user_callback([](int signo){
        std::cerr << "Signal received in main thread: " << signo << std::endl;
    });

    app::atmolyt application(argc, argv);

    // Exit early if --help, --view, --st was used (should_run() returns false)
    if (!application.should_run())
    {
        return 0;
    }

    // Initialize CSV logger
    app::csv_logger logger(application.get_log_path());

    // Clear display on startup
    if (!application.get_displays().empty()) {
        application.get_displays()[0]->clear();
    }

    // Previous values to detect changes
    std::string prev_co2_value = "";
    std::string prev_temp_value = "";
    std::string prev_hum_value = "";

    while (!signal_handler::shutdown_requested())
    {
        signal_handler::poll_and_handle();

        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
        std::string timestamp = ss.str();

        double co2_ppm = -1, temp_c = -999, press_pa = -1, humidity_rh = -1;
        
        // Async sensor reading
        auto gas_future = std::async(std::launch::async, read_gas_sensors_async, 
                                      std::cref(application.get_gas_sensors()));
        auto env_future = std::async(std::launch::async, read_env_sensors_async, 
                                      std::cref(application.get_environmental_sensors()));
        
        auto gas_data = gas_future.get();
        auto env_data = env_future.get();
        
        if (gas_data.has_value()) {
            co2_ppm = gas_data->co2_ppm;
            temp_c = gas_data->temp_c;
            humidity_rh = gas_data->humidity_rh;
        }
        
        if (env_data.has_value()) {
            press_pa = env_data->press_pa;
            if (temp_c == -999) {
                temp_c = env_data->temp_c;
            }
        }
        
        if (!application.get_displays().empty()) {
            auto& display = application.get_displays()[0];

            std::string co2_value = gas_data.has_value() ? 
                std::to_string(static_cast<int>(co2_ppm)) : "--";
            std::string temp_value = (temp_c != -999) ? 
                std::to_string(temp_c).substr(0, 4) : "--";
            std::string hum_value = gas_data.has_value() ? 
                std::to_string(static_cast<int>(humidity_rh)) : "--";

            if (co2_value != prev_co2_value || temp_value != prev_temp_value || hum_value != prev_hum_value) {
                display->clear();
                
                std::string display_text = "CO2\n" + co2_value + "\nT:" + temp_value + "\nH:" + hum_value;
                display->display_text(display_text, 0, 0, 3, true);
                
                prev_co2_value = co2_value;
                prev_temp_value = temp_value;
                prev_hum_value = hum_value;
            }
        }
        
        logger.log_async(co2_ppm, temp_c, press_pa, humidity_rh, timestamp);

        std::this_thread::sleep_for(std::chrono::seconds(5));
    }

    std::cerr << "Shutting down due to signal" << std::endl;

    // Clear display on shutdown
    if (!application.get_displays().empty()) {
        application.get_displays()[0]->clear();
    }

    return 0;
}

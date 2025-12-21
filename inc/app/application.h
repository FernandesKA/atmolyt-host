/**
 * @file application.h
 * @author FernandesKA (fernandes.kir@yandex.ru)
 * @brief  Application main class
 * @version 0.1
 * @date 2025-12-06
 *
 * @copyright Copyright (c) 2025
 *
 */

#pragma once

#include <boost/program_options.hpp>

#include "config/config_loader.h"
#include "connections/connection_iface.h"
#include "peripheral/peripheral_factory.h"
#include <memory>
#include <vector>

namespace app
{
    class atmolyt
    {
    public:
        atmolyt(int argc, char *argv[]);
        ~atmolyt();

        // Check if app should run main loop (false if --help, --view, --st was used)
        bool should_run() const { return should_run_; }

        // Getters for peripherals
        const std::vector<std::unique_ptr<peripherals::environmental_sensor_iface>>& get_environmental_sensors() const { return environmental_sensors_; }
        const std::vector<std::unique_ptr<peripherals::gas_sensor_iface>>& get_gas_sensors() const { return gas_sensors_; }
        const std::vector<std::unique_ptr<peripherals::display_iface>>& get_displays() const { return displays_; }
        const std::vector<std::unique_ptr<peripherals::rtc_iface>>& get_rtcs() const { return rtcs_; }

    private:

        int parse_inarg(int, char**);

        // Load configuration and instantiate peripherals
        bool load_and_create_peripherals();

    private:
        bool is_periphery_init = false;
        bool should_run_ = true;
        std::string config_path_ = "./config/atmolyt.json";

        // storage for live connections and peripheral objects
        std::vector<std::unique_ptr<connections::connection_iface<uint8_t>>> connections_;
        std::vector<std::unique_ptr<peripherals::environmental_sensor_iface>> environmental_sensors_;
        std::vector<std::unique_ptr<peripherals::gas_sensor_iface>> gas_sensors_;
        std::vector<std::unique_ptr<peripherals::display_iface>> displays_;
        std::vector<std::unique_ptr<peripherals::rtc_iface>> rtcs_;

    };

};
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

    };

};
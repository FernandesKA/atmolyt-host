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

    private:

        int parse_inarg(int, char**);

        // Load configuration and instantiate peripherals
        bool load_and_create_peripherals();

    private:
        bool is_periphery_init = false;

        // storage for live connections and peripheral objects
        std::vector<std::unique_ptr<connections::connection_iface<uint8_t>>> connections_;
        std::vector<std::unique_ptr<peripherals::environmental_sensor_iface>> environmental_sensors_;

    };

};
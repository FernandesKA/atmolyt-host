/**
 * @file application.cpp
 * @author FernandesKA (fernandes.kir@yandex.ru)
 * @brief  Application main class implementation
 * @version 0.1
 * @date 2025-12-06
 *
 * @copyright Copyright (c) 2025
 *
 */

#include "app/application.h"

#include "config/config_loader.h"
#include "connections/i2c_connection.h"
#include "connections/mock_connection.h"
#include "peripheral/bme280.h"
#include "peripheral/peripheral_factory.h"
#include "self_test/self_test.h"

#include <iostream>
#include <cerrno>
#include <boost/program_options.hpp>

using namespace peripherals;

namespace app
{
    atmolyt::atmolyt(int argc, char *argv[])
        : should_run_(true)
    {
        int rc = parse_inarg(argc, argv);
        
        // rc = 1 means --help/--view/--st was shown, don't run main loop
        // rc = 0 means normal operation, initialize peripherals
        // rc < 0 means error
        if (rc == 1)
        {
            should_run_ = false;
            is_periphery_init = false;
        }
        else if (rc == 0)
        {
            is_periphery_init = true;
            should_run_ = true;
            if (!load_and_create_peripherals())
                std::cerr << "Warning: failed to create peripherals from config" << std::endl;
        }
        else
        {
            should_run_ = false;
            is_periphery_init = false;
        }
    }

    atmolyt::~atmolyt()
    {
        // ensure proper deinitialization
        for (auto &sensor : environmental_sensors_)
        {
            if (sensor)
                sensor->deinitialize();
        }
        for (auto &conn : connections_)
        {
            if (conn)
                conn->deinitialize();
        }
    }

    int atmolyt::parse_inarg(int argc, char **argv)
    {
        namespace po = boost::program_options;

        if (argc < 1 || !argv)
            return -EINVAL;

        po::options_description desc("Allowed options");
        desc.add_options()
            ("help,h", po::bool_switch()->default_value(false), "produce help message")
            ("view,v", po::bool_switch()->default_value(false), "view params from config file")
            ("st,s", po::bool_switch()->default_value(false), "begin self testing hardware")
            ("st-config", po::value<std::string>()->default_value("./config/atmolyt.json"), "path to config for self-test")
            ("st-json", po::bool_switch()->default_value(false), "output self-test results as JSON");

        po::variables_map vm;
        try
        {
            po::store(po::parse_command_line(argc, argv, desc), vm);
            po::notify(vm);
        }
        catch (const po::error &e)
        {
            std::cerr << "Argument parsing error: " << e.what() << "\n";
            std::cerr << desc << std::endl;
            return -EINVAL;
        }

        if (vm["help"].as<bool>())
        {
            std::cout << desc << "\n";
            return 1;
        }

        if (vm["view"].as<bool>())
        {
            // TODO: implement viewing config file parameters
            std::cout << "View config requested\n";
            return 1;
        }

        if (vm["st"].as<bool>())
        {
            std::cout << "Self-test requested\n";
            std::string cfg = vm["st-config"].as<std::string>();
            bool json_out = vm["st-json"].as<bool>();
            // run self-test sequence (will load config and exercise peripherals)
            st::self_test tester(cfg, json_out);
            int rc = tester.run();
            if (rc != 0)
            {
                std::cerr << "Self-test returned: " << rc << "\n";
            }
            return 1;
        }

        return 0;
    }

    bool atmolyt::load_and_create_peripherals()
    {
        config::AppConfig cfg;
        // try local config first, then /etc
        if (!config::load_config("./config/atmolyt.json", cfg))
        {
            config::load_config("/etc/atmolyt/atmolyt.json", cfg);
        }

        for (auto &p : cfg.peripherals)
        {
            std::string conn = p.connection;
            std::string type = p.type;
            uint8_t addr = p.address;

#ifdef TARGET_HOST
            // use mock connection for host
            auto mock = std::make_unique<connections::mock_addressable_connection>("mock");
            mock->initialize();
            connections_.push_back(std::move(mock));
            auto *conn_ptr = connections_.back().get();
            try {
                auto sensor = peripheral_factory::create_environmental_sensor(peripheral_factory::string_to_type(type), static_cast<connections::addressable_connection_iface<uint8_t>*>(conn_ptr), addr);
                if (sensor)
                {
                    sensor->initialize();
                    environmental_sensors_.push_back(std::move(sensor));
                }
            } catch(...) {
                std::cerr << "Failed to create sensor: " << type << std::endl;
            }
#else
            if (conn == "i2c")
            {
                auto i2c = std::make_unique<connections::i2c_connection>(p.device.empty() ? "/dev/i2c-2" : p.device);
                if (i2c->initialize() != connections::Status::Success)
                {
                    std::cerr << "Failed to init i2c: " << p.device << std::endl;
                    continue;
                }
                connections_.push_back(std::move(i2c));
                auto *conn_ptr = connections_.back().get();
                try {
                    auto sensor = peripheral_factory::create_environmental_sensor(peripheral_factory::string_to_type(type), static_cast<connections::addressable_connection_iface<uint8_t>*>(conn_ptr), addr);
                    if (sensor)
                    {
                        sensor->initialize();
                        environmental_sensors_.push_back(std::move(sensor));
                    }
                } catch(...) {
                    std::cerr << "Failed to create sensor: " << type << std::endl;
                }
            }
#endif
        }

        return true;
    }

}
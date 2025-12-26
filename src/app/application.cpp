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
#include "peripheral/peripheral_iface.h"
#include "self_test/self_test.h"

#ifdef USE_BOOST
    #include <boost/program_options.hpp>
#else
    #include "config/cmdline_parser.h"
#endif

#include <iostream>
#include <cerrno>

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
        for (auto &sensor : gas_sensors_)
        {
            if (sensor)
                sensor->deinitialize();
        }
        for (auto &display : displays_)
        {
            if (display)
                display->deinitialize();
        }
        for (auto &rtc : rtcs_)
        {
            if (rtc)
                rtc->deinitialize();
        }
        for (auto &conn : connections_)
        {
            if (conn)
                conn->deinitialize();
        }
    }

    int atmolyt::parse_inarg(int argc, char **argv)
    {
        if (argc < 1 || !argv)
            return -EINVAL;

#ifdef USE_BOOST
        namespace po = boost::program_options;

        po::options_description desc("Allowed options");
        desc.add_options()
            ("help,h", po::bool_switch()->default_value(false), "produce help message")
            ("view,v", po::bool_switch()->default_value(false), "view params from config file")
            ("config,c", po::value<std::string>()->default_value("./config/atmolyt.json"), "path to config file")
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

        config_path_ = vm["config"].as<std::string>();

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

#else  // Fallback without boost

        cmdline::OptionsDescription desc("Allowed options");
        desc.add_option("help", "h", "produce help message", "flag");
        desc.add_option("view", "v", "view params from config file", "flag");
        desc.add_option("config", "c", "path to config file", "value", "./config/atmolyt.json");
        desc.add_option("st", "s", "begin self testing hardware", "flag");
        desc.add_option("st-config", "", "path to config for self-test", "value", "./config/atmolyt.json");
        desc.add_option("st-json", "", "output self-test results as JSON", "flag");

        cmdline::CommandLineParser parser(desc);
        cmdline::VariablesMap vm;
        int parse_rc = parser.parse(argc, argv, vm);
        
        if (parse_rc != 0 || vm.has("help")) {
            desc.print(std::cout);
            if (parse_rc != 0 && !vm.has("help")) {
                return -EINVAL;
            }
            return (vm.has("help")) ? 1 : -EINVAL;
        }

        config_path_ = vm.get_string("config", "./config/atmolyt.json");

        if (vm.get_bool("view"))
        {
            // TODO: implement viewing config file parameters
            std::cout << "View config requested\n";
            return 1;
        }

        if (vm.get_bool("st"))
        {
            std::cout << "Self-test requested\n";
            std::string cfg = vm.get_string("st-config", "./config/atmolyt.json");
            bool json_out = vm.get_bool("st-json");
            // run self-test sequence (will load config and exercise peripherals)
            st::self_test tester(cfg, json_out);
            int rc = tester.run();
            if (rc != 0)
            {
                std::cerr << "Self-test returned: " << rc << "\n";
            }
            return 1;
        }

#endif

        return 0;
    }

    bool atmolyt::load_and_create_peripherals()
    {
        // try the specified config path
        if (!config::load_config(config_path_, config_))
        {
            std::cerr << "Failed to load config from " << config_path_ << std::endl;
            return false;
        }

        for (auto &p : config_.peripherals)
        {
            std::string conn = p.connection;
            std::string type = p.type;
            uint8_t addr = p.address;

#if TARGET_HOST
            // use mock connection for host
            auto mock = std::make_unique<connections::mock_addressable_connection>("mock");
            mock->initialize();
            connections_.push_back(std::move(mock));
            auto *conn_ptr = connections_.back().get();
            try {
                peripherals::PeripheralType ptype = peripheral_factory::string_to_type(type);
                if (ptype == peripherals::PeripheralType::BME280 ||
                    ptype == peripherals::PeripheralType::BMP280 ||
                    ptype == peripherals::PeripheralType::DHT22) {
                    auto sensor = peripheral_factory::create_environmental_sensor(ptype, static_cast<connections::addressable_connection_iface<uint8_t>*>(conn_ptr), addr);
                    if (sensor)
                    {
                        sensor->initialize();
                        environmental_sensors_.push_back(std::move(sensor));
                    }
                } else if (ptype == peripherals::PeripheralType::SCD41 ||
                           ptype == peripherals::PeripheralType::SGP41) {
                    auto sensor = peripheral_factory::create_gas_sensor(ptype, static_cast<connections::addressable_connection_iface<uint8_t>*>(conn_ptr), addr);
                    if (sensor)
                    {
                        sensor->initialize();
                        gas_sensors_.push_back(std::move(sensor));
                    }
                } else if (ptype == peripherals::PeripheralType::SSD1306) {
                    std::cout << "SSD1306 conn: " << conn << std::endl;
                    connections::addressable_connection_iface<uint8_t>* display_conn = nullptr;
                    if (conn != "fb") {
                        // I2C mode
                        auto i2c = std::make_unique<connections::i2c_connection>(p.device.empty() ? "/dev/i2c-2" : p.device);
                        if (i2c->initialize() != connections::Status::Success) {
                            std::cerr << "Failed to init i2c for display: " << p.device << std::endl;
                            continue;
                        }
                        connections_.push_back(std::move(i2c));
                        display_conn = connections_.back().get();
                    }
                    // For fb, display_conn remains nullptr
                    auto display = peripheral_factory::create_display(ptype, display_conn, addr);
                    if (display)
                    {
                        display->initialize();
                        displays_.push_back(std::move(display));
                    }
                } else if (ptype == peripherals::PeripheralType::DS3231) {
                    auto rtc = peripheral_factory::create_rtc(ptype, static_cast<connections::addressable_connection_iface<uint8_t>*>(conn_ptr), addr);
                    if (rtc)
                    {
                        rtc->initialize();
                        rtcs_.push_back(std::move(rtc));
                    }
                } else {
                    std::cerr << "Unsupported sensor type: " << type << std::endl;
                }
            } catch(...) {
                std::cerr << "Failed to create sensor: " << type << std::endl;
            }
#else
            connections::addressable_connection_iface<uint8_t>* conn_ptr = nullptr;
            if (conn == "i2c")
            {
                auto i2c = std::make_unique<connections::i2c_connection>(p.device.empty() ? "/dev/i2c-1" : p.device);
                if (i2c->initialize() != connections::Status::Success)
                {
                    std::cerr << "Failed to init i2c: " << p.device << std::endl;
                    continue;
                }
                connections_.push_back(std::move(i2c));
                conn_ptr = static_cast<connections::addressable_connection_iface<uint8_t>*>(connections_.back().get());
            }
            else if (conn == "fb")
            {
                // framebuffer, no connection
                conn_ptr = nullptr;
            }
            else
            {
                std::cerr << "Unsupported connection type: " << conn << std::endl;
                continue;
            }
            try {
                peripherals::PeripheralType ptype = peripheral_factory::string_to_type(type);
                if (ptype == peripherals::PeripheralType::BME280 ||
                    ptype == peripherals::PeripheralType::BMP280 ||
                    ptype == peripherals::PeripheralType::DHT22) {
                    auto sensor = peripheral_factory::create_environmental_sensor(ptype, conn_ptr, addr);
                    if (sensor)
                    {
                        sensor->initialize();
                        environmental_sensors_.push_back(std::move(sensor));
                    }
                } else if (ptype == peripherals::PeripheralType::SCD41 ||
                           ptype == peripherals::PeripheralType::SGP41) {
                    auto sensor = peripheral_factory::create_gas_sensor(ptype, conn_ptr, addr);
                    if (sensor)
                    {
                        sensor->initialize();
                        gas_sensors_.push_back(std::move(sensor));
                    }
                } else if (ptype == peripherals::PeripheralType::SSD1306) {
                    auto display = peripheral_factory::create_display(ptype, conn_ptr, addr);
                    if (display)
                    {
                        display->initialize();
                        displays_.push_back(std::move(display));
                    }
                } else if (ptype == peripherals::PeripheralType::DS3231) {
                    auto rtc = peripheral_factory::create_rtc(ptype, conn_ptr, addr);
                    if (rtc)
                    {
                        rtc->initialize();
                        rtcs_.push_back(std::move(rtc));
                    }
                } else {
                    std::cerr << "Unsupported sensor type: " << type << std::endl;
                }
            } catch(...) {
                std::cerr << "Failed to create sensor: " << type << std::endl;
            }
#endif
        }

        // Display initialization success message
        for (auto& display : displays_) {
            display->display_text("Initialization successful", 0, 0);
        }

        return true;
    }

}
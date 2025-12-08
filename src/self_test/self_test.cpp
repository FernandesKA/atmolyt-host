/**
 * @file self_test.cpp
 * @brief Self-test implementation: exercises connections and peripherals
 */

#include "self_test/self_test.h"

#include "config/config_loader.h"
#include "peripheral/peripheral_factory.h"
#include "connections/mock_connection.h"
#include "connections/i2c_connection.h"

#include <iostream>
#include <vector>
#include <memory>

using namespace peripherals;

namespace st
{
    self_test::self_test(const std::string &config_path, bool json_output)
        : config_path_(config_path), json_output_(json_output)
    {
    }

    self_test::~self_test() = default;

    static std::unique_ptr<connections::addressable_connection_iface<uint8_t>>
    make_connection(const config::PeripheralSpec &spec)
    {
        if (spec.connection == "mock")
        {
            return std::make_unique<connections::mock_addressable_connection>("mock");
        }
        if (spec.connection == "i2c")
        {
            auto dev = spec.device.empty() ? std::string("/dev/i2c-2") : spec.device;
            return std::make_unique<connections::i2c_connection>(dev);
        }

        return nullptr;
    }

    int self_test::run()
    {
        config::AppConfig cfg;
        if (!config::load_config(config_path_, cfg))
        {
            std::cout << "Self-test: failed to load config: " << config_path_ << "\n";
            std::cout << "Trying /etc/atmolyt/atmolyt.json\n";
            if (!config::load_config("/etc/atmolyt/atmolyt.json", cfg))
            {
                std::cerr << "Self-test: no config available, aborting\n";
                return -1;
            }
        }

        int overall_failures = 0;

        struct per_result { std::string type; std::string connection; int address; bool ok; float t; float h; float p; std::string message; };
        std::vector<per_result> results;

        for (const auto &p : cfg.peripherals)
        {
            std::cout << "--- Testing peripheral: type='" << p.type << "' conn='" << p.connection << "' addr='" << int(p.address) << "' device='" << p.device << "' ---\n";

            auto conn = make_connection(p);
            if (!conn)
            {
                std::cerr << "  Unsupported connection type: " << p.connection << "\n";
                results.push_back({p.type, p.connection, int(p.address), false, 0.0f, 0.0f, 0.0f, "unsupported connection"});
                ++overall_failures;
                continue;
            }

            auto init_status = conn->initialize();
            if (init_status != connections::Status::Success)
            {
                std::cerr << "  Connection init failed\n";
                results.push_back({p.type, p.connection, int(p.address), false, 0.0f, 0.0f, 0.0f, "connection init failed"});
                ++overall_failures;
                continue;
            }

            // create peripheral instance
            peripherals::PeripheralType t = peripheral_factory::string_to_type(p.type);
            std::unique_ptr<peripherals::environmental_sensor_iface> sensor;
            try {
                sensor = peripheral_factory::create_environmental_sensor(t, conn.get(), p.address);
            } catch (...) {
                sensor = nullptr;
            }

            if (!sensor)
            {
                std::cerr << "  Failed to create sensor object for type: " << p.type << "\n";
                results.push_back({p.type, p.connection, int(p.address), false, 0.0f, 0.0f, 0.0f, "create sensor failed"});
                conn->deinitialize();
                ++overall_failures;
                continue;
            }

            // initialize sensor first (mock sensor reports connected after init)
            auto st = sensor->initialize();
            if (st != peripherals::Status::Success)
            {
                std::cerr << "  Sensor initialize failed\n";
                results.push_back({p.type, p.connection, int(p.address), false, 0.0f, 0.0f, 0.0f, "sensor init failed"});
                sensor->deinitialize();
                conn->deinitialize();
                ++overall_failures;
                continue;
            }

            // check connectivity (for real sensors this reads ID, for mock it checks initialized_)
            bool connected = sensor->is_connected();
            std::cout << "  is_connected: " << (connected ? "yes" : "no") << "\n";
            if (!connected)
            {
                std::cerr << "  Sensor not responding\n";
                sensor->deinitialize();
                conn->deinitialize();
                ++overall_failures;
                continue;
            }

            peripherals::combined_env_data data{};
            st = sensor->read_data(data);
            if (st != peripherals::Status::Success)
            {
                std::cerr << "  Sensor read_data failed (status=" << int(st) << ")\n";
                results.push_back({p.type, p.connection, int(p.address), false, 0.0f, 0.0f, 0.0f, "read failed"});
                sensor->deinitialize();
                conn->deinitialize();
                ++overall_failures;
                continue;
            }

            std::cout << "  Readings: T=" << data.temperature.celsius << "C H=" << data.humidity.relative_humidity << "% P=" << data.pressure.pascals << "Pa\n";

            // Try reset
            auto rst = sensor->reset();
            std::cout << "  reset: " << (rst == peripherals::Status::Success ? "ok" : "failed") << "\n";

            results.push_back({p.type, p.connection, int(p.address), true, data.temperature.celsius, data.humidity.relative_humidity, data.pressure.pascals, "ok"});

            sensor->deinitialize();
            conn->deinitialize();
        }

        // Output JSON if requested
        if (json_output_)
        {
            std::cout << "{";
            std::cout << "\"summary\":{\"failures\":" << overall_failures << "},\"peripherals\":[";
            for (size_t i = 0; i < results.size(); ++i)
            {
                auto &r = results[i];
                std::cout << "{";
                std::cout << "\"type\":\"" << r.type << "\",";
                std::cout << "\"connection\":\"" << r.connection << "\",";
                std::cout << "\"address\":\"" << r.address << "\",";
                std::cout << "\"ok\":" << (r.ok ? "true" : "false") << ",";
                std::cout << "\"temperature_c\":" << r.t << ",";
                std::cout << "\"humidity_pct\":" << r.h << ",";
                std::cout << "\"pressure_pa\":" << r.p << ",";
                std::cout << "\"message\":\"" << r.message << "\"";
                std::cout << "}";
                if (i + 1 < results.size()) std::cout << ",";
            }
            std::cout << "]}" << std::endl;
        }

        if (overall_failures == 0)
        {
            std::cout << "Self-test: ALL PASSED\n";
            return 0;
        }
        else
        {
            std::cerr << "Self-test: " << overall_failures << " failures\n";
            return overall_failures;
        }
    }

} // namespace st

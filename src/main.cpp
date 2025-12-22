#include "app/application.h"
#include "app/signal_handler.h"
#include <iostream>
#include <thread>
#include <chrono>

using app::signal_handler;

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

    // Clear display on startup
    if (!application.get_displays().empty()) {
        application.get_displays()[0]->clear();
    }

    while (!signal_handler::shutdown_requested())
    {
        signal_handler::poll_and_handle();

        // Read sensors and display data
        if (!application.get_displays().empty()) {
            auto& display = application.get_displays()[0];
            display->clear();

            std::string co2_str = "CO2: -- ppm";
            std::string temp_str = "Temp: -- C";
            std::string press_str = "Press: -- hPa";
            std::string time_str = "Time: --:--:--";
            std::string date_str = "Date: --/--/--";

            // Read gas sensors (SCD41)
            for (auto& sensor : application.get_gas_sensors()) {
                peripherals::gas_data data;
                if (sensor->read_data(data) == peripherals::Status::Success) {
                    co2_str = "CO2: " + std::to_string(static_cast<int>(data.co2_ppm)) + " ppm";
                    break; // Take first successful
                }
            }

            // Read environmental sensors (BME280/BMP280)
            for (auto& sensor : application.get_environmental_sensors()) {
                peripherals::combined_env_data data;
                if (sensor->read_data(data) == peripherals::Status::Success) {
                    temp_str = "Temp: " + std::to_string(data.temperature.celsius).substr(0, 4) + " C";
                    press_str = "Press: " + std::to_string(static_cast<int>(data.pressure.pascals / 100.0)) + " hPa";
                    break; // Take first successful
                }
            }

            // Read RTC (DS3231)
            for (auto& rtc : application.get_rtcs()) {
                peripherals::time_data time;
                if (rtc->read_data(time) == peripherals::Status::Success) {
                    char buf[12];
                    std::snprintf(buf, sizeof(buf), "%02d:%02d:%02d", time.hour, time.minute, time.second);
                    time_str = "Time: " + std::string(buf);
                    std::snprintf(buf, sizeof(buf), "%02d/%02d/%02d", time.day, time.month, time.year % 100);
                    date_str = "Date: " + std::string(buf);
                    break; // Take first successful
                }
            }

            std::string text = co2_str + "\n" + temp_str + "\n" + press_str + "\n" + time_str + "\n" + date_str;

            display->display_text(text);
        }

        std::this_thread::sleep_for(std::chrono::seconds(5)); // Update every 5 seconds
    }

    std::cerr << "Shutting down due to signal" << std::endl;

    // Clear display on shutdown
    if (!application.get_displays().empty()) {
        application.get_displays()[0]->clear();
    }

    return 0;
}

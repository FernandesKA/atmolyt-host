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

    while (!signal_handler::shutdown_requested())
    {
        signal_handler::poll_and_handle();

        // Read sensors and display data
        if (!application.get_displays().empty()) {
            auto& display = application.get_displays()[0];
            display->clear();

            std::string text = "Atmolyt Data\n";

            // Read environmental sensors
            for (auto& sensor : application.get_environmental_sensors()) {
                peripherals::combined_env_data data;
                if (sensor->read_data(data) == peripherals::Status::Success) {
                    text += "Temp: " + std::to_string(data.temperature.celsius) + "C\n";
                    text += "Hum: " + std::to_string(data.humidity.relative_humidity) + "%\n";
                    text += "Press: " + std::to_string(data.pressure.pascals / 100.0) + " hPa\n";
                }
            }

            // Read gas sensors
            for (auto& sensor : application.get_gas_sensors()) {
                peripherals::gas_data data;
                if (sensor->read_data(data) == peripherals::Status::Success) {
                    text += "CO2: " + std::to_string(data.co2_ppm) + " ppm\n";
                    text += "TVOC: " + std::to_string(data.tvoc_ppb) + " ppb\n";
                }
            }

            // Read RTC
            for (auto& rtc : application.get_rtcs()) {
                peripherals::time_data time;
                if (rtc->read_data(time) == peripherals::Status::Success) {
                    text += "Time: " + std::to_string(time.hour) + ":" + std::to_string(time.minute) + ":" + std::to_string(time.second) + "\n";
                    text += "Date: " + std::to_string(time.day) + "/" + std::to_string(time.month) + "/" + std::to_string(time.year) + "\n";
                }
            }

            display->display_text(text);
        }

        std::this_thread::sleep_for(std::chrono::seconds(5)); // Update every 5 seconds
    }

    std::cerr << "Shutting down due to signal" << std::endl;
    return 0;
}

#include "app/application.h"
#include "app/signal_handler.h"
#include "app/csv_logger.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <iomanip>
#include <sstream>

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

        // Read sensors and display data
        double co2_ppm = -1, temp_c = -999, press_pa = -1, humidity_rh = -1;
        std::string timestamp;
        
        // Get current timestamp
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
        timestamp = ss.str();
        
        if (!application.get_displays().empty()) {
            auto& display = application.get_displays()[0];

            std::string co2_value = "--";
            std::string temp_value = "--";
            std::string hum_value = "--";

            for (auto& sensor : application.get_gas_sensors()) {
                peripherals::gas_data data;
                if (sensor->read_data(data) == peripherals::Status::Success) {
                    co2_ppm = data.co2_ppm;
                    temp_c = data.temperature_c;
                    humidity_rh = data.humidity_rh;
                    co2_value = std::to_string(static_cast<int>(co2_ppm));
                    temp_value = std::to_string(temp_c).substr(0, 4);
                    hum_value = std::to_string(static_cast<int>(humidity_rh));
                    break; // Take first successful
                }
            }

            for (auto& sensor : application.get_environmental_sensors()) {
                peripherals::combined_env_data data;
                if (sensor->read_data(data) == peripherals::Status::Success) {
                    press_pa = data.pressure.pascals;
                    // Use BMP280 temperature only if not available from SCD41
                    if (temp_c == -999) {
                        temp_c = data.temperature.celsius;
                        temp_value = std::to_string(temp_c).substr(0, 4);
                    }
                    break;
                }
            }

            if (co2_value != prev_co2_value || temp_value != prev_temp_value || hum_value != prev_hum_value) {
                display->clear();
                
                std::string line1 = "CO2";
                std::string line2 = co2_value;
                std::string line3 = "T:" + temp_value;
                std::string line4 = "H:" + hum_value;
                
                std::string display_text = line1 + "\n" + line2 + "\n" + line3 + "\n" + line4;
                display->display_text(display_text, 0, 0, 3, true);
                
                prev_co2_value = co2_value;
                prev_temp_value = temp_value;
                prev_hum_value = hum_value;
            }
        }
        
        // Log data to CSV
        logger.log(co2_ppm, temp_c, press_pa, humidity_rh, timestamp);

        std::this_thread::sleep_for(std::chrono::seconds(5)); // Update every 5 seconds
    }

    std::cerr << "Shutting down due to signal" << std::endl;

    // Clear display on shutdown
    if (!application.get_displays().empty()) {
        application.get_displays()[0]->clear();
    }

    return 0;
}

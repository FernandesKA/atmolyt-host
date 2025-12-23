#include "config/config_loader.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <iostream>

namespace config {

using boost::property_tree::ptree;

bool load_config(const std::string &path, AppConfig &out)
{
    ptree root;
    try {
        boost::property_tree::read_json(path, root);
    }
    catch (const std::exception &e) {
        std::cerr << "Failed to read config: " << e.what() << std::endl;
        return false;
    }

    out.peripherals.clear();
    out.log_path = root.get<std::string>("log_path", "atmolyt_data.csv");
    
    for (auto &item : root.get_child("peripherals")) {
        PeripheralSpec spec;
        ptree node = item.second;
        try {
            spec.connection = node.get<std::string>("connection", "i2c");
            spec.type = node.get<std::string>("type", "bme280");
            spec.device = node.get<std::string>("device", "");
            std::string addr = node.get<std::string>("address", "0x76");
            try {
                if (addr.rfind("0x", 0) == 0 || addr.rfind("0X", 0) == 0)
                    spec.address = static_cast<uint8_t>(std::stoul(addr, nullptr, 16));
                else
                    spec.address = static_cast<uint8_t>(std::stoul(addr));
            } catch (...) { spec.address = 0x76; }

            out.peripherals.push_back(spec);
        }
        catch (...) {
            // skip invalid entry
        }
    }

    return true;
}

} // namespace config

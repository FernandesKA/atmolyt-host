/**
 * @file config_loader.cpp
 * @author FernandesKA (fernandes.kir@yandex.ru)
 * @brief  Configuration loader for the application
 * @version 0.1
 * @date 2025-12-23
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "config/config_loader.h"

#ifdef USE_BOOST
    #include <boost/property_tree/ptree.hpp>
    #include <boost/property_tree/json_parser.hpp>
#else
    #include "config/json_parser.h"
#endif

#include <iostream>

namespace config {

#ifdef USE_BOOST
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

#else  // Fallback JSON parser without boost

bool load_config(const std::string &path, AppConfig &out)
{
    auto root = json::parse_file(path);
    if (!root) {
        std::cerr << "Failed to read config: " << path << std::endl;
        return false;
    }

    if (!root->is_object()) {
        std::cerr << "Config root must be an object" << std::endl;
        return false;
    }

    out.peripherals.clear();
    out.log_path = root->get_string("log_path", "atmolyt_data.csv");
    
    auto peripherals_val = root->get("peripherals");
    if (!peripherals_val || !peripherals_val->is_array()) {
        std::cerr << "Missing or invalid 'peripherals' array in config" << std::endl;
        return false;
    }

    const auto& peripherals = peripherals_val->as_array();
    for (const auto& item : peripherals) {
        if (!item || !item->is_object()) {
            // skip invalid entry
            continue;
        }

        PeripheralSpec spec;
        try {
            spec.connection = item->get_string("connection", "i2c");
            spec.type = item->get_string("type", "bme280");
            spec.device = item->get_string("device", "");
            
            std::string addr = item->get_string("address", "0x76");
            try {
                if (addr.rfind("0x", 0) == 0 || addr.rfind("0X", 0) == 0)
                    spec.address = static_cast<uint8_t>(std::stoul(addr, nullptr, 16));
                else
                    spec.address = static_cast<uint8_t>(std::stoul(addr));
            } catch (...) { 
                spec.address = 0x76; 
            }

            out.peripherals.push_back(spec);
        }
        catch (...) {
            // skip invalid entry
        }
    }

    return true;
}

#endif

} // namespace config

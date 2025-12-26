/**
 * @file cmdline_parser.cpp
 * @author FernandesKA (fernandes.kir@yandex.ru)
 * @brief  Simple command line parser implementation
 * @version 0.1
 * @date 2025-12-27
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "config/cmdline_parser.h"
#include <iomanip>

namespace cmdline {

void OptionsDescription::add_option(const std::string& name, const std::string& short_name,
                                    const std::string& description, const std::string& value_type,
                                    const std::string& default_val) {
    Option opt;
    opt.name = name;
    opt.short_name = short_name;
    opt.description = description;
    opt.value_type = value_type;
    opt.default_val = default_val;
    options_.push_back(opt);
}

void OptionsDescription::print(std::ostream& os) const {
    if (!description_.empty()) {
        os << description_ << ":\n";
    }
    
    for (const auto& opt : options_) {
        os << "  ";
        if (!opt.short_name.empty()) {
            os << "-" << opt.short_name << ", ";
        }
        os << "--" << opt.name;
        
        if (opt.value_type == "value") {
            os << " arg";
        }
        
        os << std::setw(30 - opt.name.length()) << " ";
        os << opt.description;
        
        if (!opt.default_val.empty()) {
            os << " (default: " << opt.default_val << ")";
        }
        os << "\n";
    }
}

const OptionsDescription::Option* OptionsDescription::find_option(const std::string& name) const {
    // Try full name first
    for (const auto& opt : options_) {
        if (opt.name == name) {
            return &opt;
        }
    }
    // Try short name
    for (const auto& opt : options_) {
        if (opt.short_name == name) {
            return &opt;
        }
    }
    return nullptr;
}

const OptionsDescription::Option* CommandLineParser::find_option(const std::string& name) const {
    return desc_.find_option(name);
}

int CommandLineParser::parse(int argc, char* argv[], VariablesMap& vm) const {
    if (argc < 1 || !argv) {
        return -1;
    }

    // Set defaults
    for (const auto& opt : desc_.options()) {
        if (!opt.default_val.empty()) {
            vm.set(opt.name, opt.default_val);
        }
    }

    // Parse arguments
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        
        if (arg == "--help" || arg == "-h") {
            vm.set("help", "true");
            continue;
        }

        // Parse long option
        if (arg.substr(0, 2) == "--") {
            std::string opt_name = arg.substr(2);
            
            // Check for value format: --name=value
            size_t eq_pos = opt_name.find('=');
            std::string value;
            if (eq_pos != std::string::npos) {
                value = opt_name.substr(eq_pos + 1);
                opt_name = opt_name.substr(0, eq_pos);
            }

            const auto* opt = find_option(opt_name);
            if (!opt) {
                std::cerr << "Unknown option: --" << opt_name << std::endl;
                return -1;
            }

            if (opt->value_type == "value") {
                if (!value.empty()) {
                    vm.set(opt->name, value);
                } else if (i + 1 < argc) {
                    vm.set(opt->name, argv[++i]);
                } else {
                    std::cerr << "Option --" << opt_name << " requires a value" << std::endl;
                    return -1;
                }
            } else {
                vm.set(opt->name, "true");
            }
        }
        // Parse short option
        else if (arg.substr(0, 1) == "-" && arg.length() > 1) {
            std::string short_name = arg.substr(1);
            
            // Check for value format: -n value
            const auto* opt = find_option(short_name);
            if (!opt) {
                std::cerr << "Unknown option: -" << short_name << std::endl;
                return -1;
            }

            if (opt->value_type == "value") {
                if (i + 1 < argc) {
                    vm.set(opt->name, argv[++i]);
                } else {
                    std::cerr << "Option -" << short_name << " requires a value" << std::endl;
                    return -1;
                }
            } else {
                vm.set(opt->name, "true");
            }
        } else {
            std::cerr << "Unexpected argument: " << arg << std::endl;
            return -1;
        }
    }

    return 0;
}

} // namespace cmdline

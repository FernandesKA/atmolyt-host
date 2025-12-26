/**
 * @file cmdline_parser.h
 * @author FernandesKA (fernandes.kir@yandex.ru)
 * @brief  Simple command line parser without boost dependency
 * @version 0.1
 * @date 2025-12-27
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#pragma once

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <sstream>
#include <iostream>

namespace cmdline {

class OptionsDescription {
public:
    struct Option {
        std::string name;
        std::string short_name;
        std::string description;
        std::string value_type; // "flag", "value", or empty
        std::string default_val;
    };

    OptionsDescription() = default;
    explicit OptionsDescription(const std::string& desc) : description_(desc) {}

    void add_option(const std::string& name, const std::string& short_name,
                   const std::string& description, const std::string& value_type = "flag",
                   const std::string& default_val = "");

    void print(std::ostream& os = std::cout) const;

    const std::vector<Option>& options() const { return options_; }
    const std::string& description() const { return description_; }
    const Option* find_option(const std::string& name) const;

private:
    std::string description_;
    std::vector<Option> options_;
};

class VariablesMap {
public:
    void set(const std::string& key, const std::string& value) {
        data_[key] = value;
    }

    bool has(const std::string& key) const {
        return data_.find(key) != data_.end();
    }

    std::string get_string(const std::string& key, const std::string& default_val = "") const {
        auto it = data_.find(key);
        return it != data_.end() ? it->second : default_val;
    }

    bool get_bool(const std::string& key, bool default_val = false) const {
        auto val = get_string(key);
        if (val.empty()) return default_val;
        return val == "true" || val == "1" || val == "yes";
    }

    int get_int(const std::string& key, int default_val = 0) const {
        try {
            return std::stoi(get_string(key));
        } catch (...) {
            return default_val;
        }
    }

private:
    std::map<std::string, std::string> data_;
};

class CommandLineParser {
public:
    explicit CommandLineParser(const OptionsDescription& desc) : desc_(desc) {}

    int parse(int argc, char* argv[], VariablesMap& vm) const;

private:
    OptionsDescription desc_;

    const OptionsDescription::Option* find_option(const std::string& name) const;
};

// Convenience functions
inline OptionsDescription make_options(const std::string& description = "") {
    return OptionsDescription(description);
}

} // namespace cmdline

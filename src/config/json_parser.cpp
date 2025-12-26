/**
 * @file json_parser.cpp
 * @author FernandesKA (fernandes.kir@yandex.ru)
 * @brief  Simple JSON parser implementation
 * @version 0.1
 * @date 2025-12-27
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "config/json_parser.h"
#include <fstream>
#include <sstream>
#include <cctype>
#include <iostream>

namespace json {

class Parser {
public:
    explicit Parser(const std::string& str) : input_(str), pos_(0) {}

    std::shared_ptr<Value> parse() {
        skip_whitespace();
        return parse_value();
    }

private:
    std::string input_;
    size_t pos_;

    char current() const { return pos_ < input_.length() ? input_[pos_] : '\0'; }
    
    char peek(int offset = 1) const {
        return pos_ + offset < input_.length() ? input_[pos_ + offset] : '\0';
    }

    void advance() { pos_++; }

    void skip_whitespace() {
        while (pos_ < input_.length() && std::isspace(input_[pos_])) {
            pos_++;
        }
    }

    std::shared_ptr<Value> parse_value() {
        skip_whitespace();
        
        char c = current();
        if (c == '{') {
            return parse_object();
        } else if (c == '[') {
            return parse_array();
        } else if (c == '"') {
            return std::make_shared<Value>(parse_string());
        } else if (c == 't' || c == 'f') {
            return parse_bool();
        } else if (c == 'n') {
            return parse_null();
        } else if (c == '-' || std::isdigit(c)) {
            return parse_number();
        }
        throw std::runtime_error("Invalid JSON value");
    }

    std::shared_ptr<Value> parse_object() {
        Object obj;
        advance(); // skip '{'
        skip_whitespace();

        if (current() == '}') {
            advance();
            return std::make_shared<Value>(obj);
        }

        while (true) {
            skip_whitespace();
            if (current() != '"') {
                throw std::runtime_error("Expected string key in object");
            }
            std::string key = parse_string();
            
            skip_whitespace();
            if (current() != ':') {
                throw std::runtime_error("Expected ':' after key");
            }
            advance();
            
            auto value = parse_value();
            obj[key] = value;
            
            skip_whitespace();
            if (current() == '}') {
                advance();
                break;
            } else if (current() == ',') {
                advance();
            } else {
                throw std::runtime_error("Expected ',' or '}' in object");
            }
        }
        return std::make_shared<Value>(obj);
    }

    std::shared_ptr<Value> parse_array() {
        Array arr;
        advance(); // skip '['
        skip_whitespace();

        if (current() == ']') {
            advance();
            return std::make_shared<Value>(arr);
        }

        while (true) {
            arr.push_back(parse_value());
            skip_whitespace();
            
            if (current() == ']') {
                advance();
                break;
            } else if (current() == ',') {
                advance();
            } else {
                throw std::runtime_error("Expected ',' or ']' in array");
            }
        }
        return std::make_shared<Value>(arr);
    }

    std::string parse_string() {
        std::string result;
        advance(); // skip opening '"'
        
        while (pos_ < input_.length() && current() != '"') {
            if (current() == '\\') {
                advance();
                if (pos_ >= input_.length()) {
                    throw std::runtime_error("Unexpected end of string");
                }
                char c = current();
                switch (c) {
                    case '"': result += '"'; break;
                    case '\\': result += '\\'; break;
                    case '/': result += '/'; break;
                    case 'b': result += '\b'; break;
                    case 'f': result += '\f'; break;
                    case 'n': result += '\n'; break;
                    case 'r': result += '\r'; break;
                    case 't': result += '\t'; break;
                    case 'u': {
                        // Simple unicode escape - just skip for now
                        advance();
                        for (int i = 0; i < 3 && pos_ < input_.length(); i++) {
                            advance();
                        }
                        continue;
                    }
                    default: result += c;
                }
                advance();
            } else {
                result += current();
                advance();
            }
        }
        
        if (current() != '"') {
            throw std::runtime_error("Unterminated string");
        }
        advance(); // skip closing '"'
        return result;
    }

    std::shared_ptr<Value> parse_number() {
        std::string num_str;
        if (current() == '-') {
            num_str += current();
            advance();
        }
        
        while (pos_ < input_.length() && (std::isdigit(current()) || current() == '.' || 
               current() == 'e' || current() == 'E' || current() == '+' || current() == '-')) {
            num_str += current();
            advance();
        }
        
        try {
            double value = std::stod(num_str);
            return std::make_shared<Value>(value);
        } catch (...) {
            throw std::runtime_error("Invalid number: " + num_str);
        }
    }

    std::shared_ptr<Value> parse_bool() {
        if (input_.substr(pos_, 4) == "true") {
            pos_ += 4;
            return std::make_shared<Value>(true);
        } else if (input_.substr(pos_, 5) == "false") {
            pos_ += 5;
            return std::make_shared<Value>(false);
        }
        throw std::runtime_error("Invalid boolean");
    }

    std::shared_ptr<Value> parse_null() {
        if (input_.substr(pos_, 4) == "null") {
            pos_ += 4;
            return std::make_shared<Value>();
        }
        throw std::runtime_error("Invalid null");
    }
};

std::shared_ptr<Value> Value::get(const std::string& key) const {
    if (type_ != Type::Object) {
        return nullptr;
    }
    auto it = object_value_.find(key);
    if (it != object_value_.end()) {
        return it->second;
    }
    return nullptr;
}

std::string Value::get_string(const std::string& key, const std::string& default_val) const {
    auto val = get(key);
    if (val && val->is_string()) {
        return val->as_string();
    }
    return default_val;
}

int Value::get_int(const std::string& key, int default_val) const {
    auto val = get(key);
    if (val && val->is_number()) {
        return val->as_int();
    }
    return default_val;
}

double Value::get_number(const std::string& key, double default_val) const {
    auto val = get(key);
    if (val && val->is_number()) {
        return val->as_number();
    }
    return default_val;
}

bool Value::get_bool(const std::string& key, bool default_val) const {
    auto val = get(key);
    if (val && val->is_bool()) {
        return val->as_bool();
    }
    return default_val;
}

std::shared_ptr<Value> parse(const std::string& json_str) {
    try {
        Parser parser(json_str);
        return parser.parse();
    } catch (const std::exception& e) {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
        return nullptr;
    }
}

std::shared_ptr<Value> parse_file(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Cannot open file: " << filepath << std::endl;
        return nullptr;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();

    return parse(buffer.str());
}

} // namespace json

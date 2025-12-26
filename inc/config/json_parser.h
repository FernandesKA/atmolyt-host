/**
 * @file json_parser.h
 * @author FernandesKA (fernandes.kir@yandex.ru)
 * @brief  Simple JSON parser without boost dependency
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
#include <optional>
#include <memory>

namespace json {

class Value;
using Object = std::map<std::string, std::shared_ptr<Value>>;
using Array = std::vector<std::shared_ptr<Value>>;

enum class Type {
    Null,
    Boolean,
    Number,
    String,
    Array,
    Object
};

class Value {
public:
    Value() : type_(Type::Null) {}
    explicit Value(bool b) : type_(Type::Boolean), bool_value_(b) {}
    explicit Value(double n) : type_(Type::Number), number_value_(n) {}
    explicit Value(int n) : type_(Type::Number), number_value_(static_cast<double>(n)) {}
    explicit Value(const std::string& s) : type_(Type::String), string_value_(s) {}
    explicit Value(std::string&& s) : type_(Type::String), string_value_(std::move(s)) {}
    explicit Value(const Array& a) : type_(Type::Array), array_value_(a) {}
    explicit Value(Array&& a) : type_(Type::Array), array_value_(std::move(a)) {}
    explicit Value(const Object& o) : type_(Type::Object), object_value_(o) {}
    explicit Value(Object&& o) : type_(Type::Object), object_value_(std::move(o)) {}

    Type type() const { return type_; }
    bool is_null() const { return type_ == Type::Null; }
    bool is_bool() const { return type_ == Type::Boolean; }
    bool is_number() const { return type_ == Type::Number; }
    bool is_string() const { return type_ == Type::String; }
    bool is_array() const { return type_ == Type::Array; }
    bool is_object() const { return type_ == Type::Object; }

    bool as_bool() const { return bool_value_; }
    double as_number() const { return number_value_; }
    int as_int() const { return static_cast<int>(number_value_); }
    const std::string& as_string() const { return string_value_; }
    const Array& as_array() const { return array_value_; }
    const Object& as_object() const { return object_value_; }

    // Get object value by key with optional default
    std::shared_ptr<Value> get(const std::string& key) const;
    std::string get_string(const std::string& key, const std::string& default_val = "") const;
    int get_int(const std::string& key, int default_val = 0) const;
    double get_number(const std::string& key, double default_val = 0.0) const;
    bool get_bool(const std::string& key, bool default_val = false) const;

private:
    Type type_;
    bool bool_value_ = false;
    double number_value_ = 0.0;
    std::string string_value_;
    Array array_value_;
    Object object_value_;
};

// Parse JSON from string
std::shared_ptr<Value> parse(const std::string& json_str);

// Parse JSON from file
std::shared_ptr<Value> parse_file(const std::string& filepath);

} // namespace json

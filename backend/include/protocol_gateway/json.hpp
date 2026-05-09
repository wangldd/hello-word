#pragma once

#include <map>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

namespace protocol_gateway {

class Json {
public:
    using Array = std::vector<Json>;
    using Object = std::map<std::string, Json>;
    using Value = std::variant<std::nullptr_t, bool, double, std::string, Array, Object>;

    Json();
    Json(std::nullptr_t);
    Json(bool value);
    Json(int value);
    Json(double value);
    Json(const char* value);
    Json(std::string value);
    Json(Array value);
    Json(Object value);

    static Json object();
    static Json array();
    static Json parse(const std::string& input);

    bool is_null() const;
    bool is_bool() const;
    bool is_number() const;
    bool is_string() const;
    bool is_array() const;
    bool is_object() const;

    bool as_bool(bool fallback = false) const;
    double as_number(double fallback = 0.0) const;
    int as_int(int fallback = 0) const;
    const std::string& as_string() const;
    std::string as_string_or(const std::string& fallback) const;
    const Array& as_array() const;
    Array& as_array();
    const Object& as_object() const;
    Object& as_object();

    bool contains(const std::string& key) const;
    const Json& at(const std::string& key) const;
    Json& operator[](const std::string& key);
    const Json& operator[](const std::string& key) const;

    std::string dump(int indent = -1) const;

private:
    Value value_;
};

class JsonError : public std::runtime_error {
public:
    explicit JsonError(const std::string& message) : std::runtime_error(message) {}
};

}  // namespace protocol_gateway

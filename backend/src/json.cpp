#include "protocol_gateway/json.hpp"

#include <cctype>
#include <cmath>
#include <iomanip>
#include <sstream>

namespace protocol_gateway {

namespace {

class Parser {
public:
    explicit Parser(const std::string& input) : input_(input) {}

    Json parse() {
        skip_ws();
        Json value = parse_value();
        skip_ws();
        if (pos_ != input_.size()) {
            throw JsonError("Unexpected trailing JSON content");
        }
        return value;
    }

private:
    Json parse_value() {
        if (match("null")) return Json(nullptr);
        if (match("true")) return Json(true);
        if (match("false")) return Json(false);
        if (peek() == '"') return Json(parse_string());
        if (peek() == '[') return parse_array();
        if (peek() == '{') return parse_object();
        return Json(parse_number());
    }

    Json parse_array() {
        consume('[');
        Json::Array values;
        skip_ws();
        if (peek() == ']') {
            consume(']');
            return values;
        }
        while (true) {
            skip_ws();
            values.push_back(parse_value());
            skip_ws();
            if (peek() == ']') {
                consume(']');
                return values;
            }
            consume(',');
        }
    }

    Json parse_object() {
        consume('{');
        Json::Object object;
        skip_ws();
        if (peek() == '}') {
            consume('}');
            return object;
        }
        while (true) {
            skip_ws();
            std::string key = parse_string();
            skip_ws();
            consume(':');
            skip_ws();
            object.emplace(std::move(key), parse_value());
            skip_ws();
            if (peek() == '}') {
                consume('}');
                return object;
            }
            consume(',');
        }
    }

    std::string parse_string() {
        consume('"');
        std::string result;
        while (pos_ < input_.size()) {
            char ch = input_[pos_++];
            if (ch == '"') return result;
            if (ch != '\\') {
                result.push_back(ch);
                continue;
            }
            if (pos_ >= input_.size()) throw JsonError("Invalid JSON escape");
            char esc = input_[pos_++];
            switch (esc) {
                case '"': result.push_back('"'); break;
                case '\\': result.push_back('\\'); break;
                case '/': result.push_back('/'); break;
                case 'b': result.push_back('\b'); break;
                case 'f': result.push_back('\f'); break;
                case 'n': result.push_back('\n'); break;
                case 'r': result.push_back('\r'); break;
                case 't': result.push_back('\t'); break;
                default: throw JsonError("Unsupported JSON escape");
            }
        }
        throw JsonError("Unterminated JSON string");
    }

    double parse_number() {
        const std::size_t start = pos_;
        if (peek() == '-') ++pos_;
        while (std::isdigit(static_cast<unsigned char>(peek()))) ++pos_;
        if (peek() == '.') {
            ++pos_;
            while (std::isdigit(static_cast<unsigned char>(peek()))) ++pos_;
        }
        if (peek() == 'e' || peek() == 'E') {
            ++pos_;
            if (peek() == '+' || peek() == '-') ++pos_;
            while (std::isdigit(static_cast<unsigned char>(peek()))) ++pos_;
        }
        if (start == pos_) throw JsonError("Expected JSON value");
        return std::stod(input_.substr(start, pos_ - start));
    }

    bool match(const char* literal) {
        std::string token(literal);
        if (input_.compare(pos_, token.size(), token) != 0) return false;
        pos_ += token.size();
        return true;
    }

    char peek() const {
        return pos_ < input_.size() ? input_[pos_] : '\0';
    }

    void consume(char expected) {
        if (peek() != expected) {
            std::ostringstream message;
            message << "Expected '" << expected << "' in JSON";
            throw JsonError(message.str());
        }
        ++pos_;
    }

    void skip_ws() {
        while (std::isspace(static_cast<unsigned char>(peek()))) ++pos_;
    }

    const std::string& input_;
    std::size_t pos_ = 0;
};

std::string escape_string(const std::string& input) {
    std::ostringstream out;
    out << '"';
    for (char ch : input) {
        switch (ch) {
            case '"': out << "\\\""; break;
            case '\\': out << "\\\\"; break;
            case '\n': out << "\\n"; break;
            case '\r': out << "\\r"; break;
            case '\t': out << "\\t"; break;
            default: out << ch; break;
        }
    }
    out << '"';
    return out.str();
}

void dump_impl(const Json& json, std::ostringstream& out, int indent, int depth) {
    const std::string pad = indent >= 0 ? std::string(depth * indent, ' ') : std::string();
    const std::string child_pad = indent >= 0 ? std::string((depth + 1) * indent, ' ') : std::string();
    if (json.is_null()) {
        out << "null";
    } else if (json.is_bool()) {
        out << (json.as_bool() ? "true" : "false");
    } else if (json.is_number()) {
        const double value = json.as_number();
        if (std::floor(value) == value) out << static_cast<long long>(value);
        else out << std::setprecision(15) << value;
    } else if (json.is_string()) {
        out << escape_string(json.as_string());
    } else if (json.is_array()) {
        out << '[';
        const auto& array = json.as_array();
        for (std::size_t i = 0; i < array.size(); ++i) {
            if (i) out << ',';
            if (indent >= 0) out << '\n' << child_pad;
            dump_impl(array[i], out, indent, depth + 1);
        }
        if (indent >= 0 && !array.empty()) out << '\n' << pad;
        out << ']';
    } else {
        out << '{';
        const auto& object = json.as_object();
        std::size_t i = 0;
        for (const auto& [key, value] : object) {
            if (i++) out << ',';
            if (indent >= 0) out << '\n' << child_pad;
            out << escape_string(key) << (indent >= 0 ? ": " : ":");
            dump_impl(value, out, indent, depth + 1);
        }
        if (indent >= 0 && !object.empty()) out << '\n' << pad;
        out << '}';
    }
}

const Json null_json;

}  // namespace

Json::Json() : value_(nullptr) {}
Json::Json(std::nullptr_t) : value_(nullptr) {}
Json::Json(bool value) : value_(value) {}
Json::Json(int value) : value_(static_cast<double>(value)) {}
Json::Json(double value) : value_(value) {}
Json::Json(const char* value) : value_(std::string(value)) {}
Json::Json(std::string value) : value_(std::move(value)) {}
Json::Json(Array value) : value_(std::move(value)) {}
Json::Json(Object value) : value_(std::move(value)) {}

Json Json::object() { return Object{}; }
Json Json::array() { return Array{}; }
Json Json::parse(const std::string& input) { return Parser(input).parse(); }

bool Json::is_null() const { return std::holds_alternative<std::nullptr_t>(value_); }
bool Json::is_bool() const { return std::holds_alternative<bool>(value_); }
bool Json::is_number() const { return std::holds_alternative<double>(value_); }
bool Json::is_string() const { return std::holds_alternative<std::string>(value_); }
bool Json::is_array() const { return std::holds_alternative<Array>(value_); }
bool Json::is_object() const { return std::holds_alternative<Object>(value_); }

bool Json::as_bool(bool fallback) const { return is_bool() ? std::get<bool>(value_) : fallback; }
double Json::as_number(double fallback) const { return is_number() ? std::get<double>(value_) : fallback; }
int Json::as_int(int fallback) const { return is_number() ? static_cast<int>(std::get<double>(value_)) : fallback; }
const std::string& Json::as_string() const { return std::get<std::string>(value_); }
std::string Json::as_string_or(const std::string& fallback) const { return is_string() ? std::get<std::string>(value_) : fallback; }
const Json::Array& Json::as_array() const { return std::get<Array>(value_); }
Json::Array& Json::as_array() { return std::get<Array>(value_); }
const Json::Object& Json::as_object() const { return std::get<Object>(value_); }
Json::Object& Json::as_object() { return std::get<Object>(value_); }

bool Json::contains(const std::string& key) const {
    return is_object() && as_object().find(key) != as_object().end();
}

const Json& Json::at(const std::string& key) const {
    const auto& object = as_object();
    auto it = object.find(key);
    if (it == object.end()) throw JsonError("Missing JSON key: " + key);
    return it->second;
}

Json& Json::operator[](const std::string& key) {
    if (!is_object()) value_ = Object{};
    return std::get<Object>(value_)[key];
}

const Json& Json::operator[](const std::string& key) const {
    if (!is_object()) return null_json;
    auto it = as_object().find(key);
    return it == as_object().end() ? null_json : it->second;
}

std::string Json::dump(int indent) const {
    std::ostringstream out;
    dump_impl(*this, out, indent, 0);
    return out.str();
}

}  // namespace protocol_gateway

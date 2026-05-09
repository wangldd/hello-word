#pragma once

#include <chrono>
#include <map>
#include <string>

#include "protocol_gateway/json.hpp"

namespace protocol_gateway {

enum class ProtocolType {
    Iec101,
    Iec102,
    Iec103,
    Iec104,
    Modbus,
    Iec61850,
};

std::string to_string(ProtocolType protocol);
ProtocolType protocol_from_string(const std::string& value);

struct ChannelConfig {
    std::string channel_id;
    std::string name;
    ProtocolType protocol;
    bool enabled = true;
    Json endpoint = Json::object();
    Json options = Json::object();
};

struct PointMapping {
    std::string mapping_id;
    ProtocolType source_protocol;
    std::string source_address;
    ProtocolType target_protocol;
    std::string target_address;
    std::string value_type = "float";
    double scale = 1.0;
    double offset = 0.0;
    std::string description;
};

struct TelemetryPoint {
    ProtocolType protocol;
    std::string address;
    Json value;
    std::string value_type = "float";
    double timestamp = 0.0;
    std::string quality = "good";
    Json metadata = Json::object();
};

struct ConversionRequest {
    ProtocolType source_protocol;
    ProtocolType target_protocol;
    Json payload = Json::object();
};

struct ConversionResult {
    TelemetryPoint source;
    TelemetryPoint target;
    Json encoded_payload = Json::object();
};

Json to_json(const ChannelConfig& channel);
Json to_json(const PointMapping& mapping);
Json to_json(const TelemetryPoint& point);
Json to_json(const ConversionResult& result);

ChannelConfig channel_from_json(const Json& json);
PointMapping mapping_from_json(const Json& json);
ConversionRequest conversion_request_from_json(const Json& json);

double unix_time_seconds();

}  // namespace protocol_gateway

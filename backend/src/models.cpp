#include "protocol_gateway/models.hpp"

#include <stdexcept>

namespace protocol_gateway {

std::string to_string(ProtocolType protocol) {
    switch (protocol) {
        case ProtocolType::Iec101: return "iec101";
        case ProtocolType::Iec102: return "iec102";
        case ProtocolType::Iec103: return "iec103";
        case ProtocolType::Iec104: return "iec104";
        case ProtocolType::Modbus: return "modbus";
        case ProtocolType::Iec61850: return "iec61850";
    }
    throw std::invalid_argument("Unsupported protocol");
}

ProtocolType protocol_from_string(const std::string& value) {
    if (value == "iec101") return ProtocolType::Iec101;
    if (value == "iec102") return ProtocolType::Iec102;
    if (value == "iec103") return ProtocolType::Iec103;
    if (value == "iec104") return ProtocolType::Iec104;
    if (value == "modbus") return ProtocolType::Modbus;
    if (value == "iec61850") return ProtocolType::Iec61850;
    throw std::invalid_argument("Unsupported protocol: " + value);
}

Json to_json(const ChannelConfig& channel) {
    Json json = Json::object();
    json["channel_id"] = channel.channel_id;
    json["name"] = channel.name;
    json["protocol"] = to_string(channel.protocol);
    json["enabled"] = channel.enabled;
    json["endpoint"] = channel.endpoint;
    json["options"] = channel.options;
    return json;
}

Json to_json(const PointMapping& mapping) {
    Json json = Json::object();
    json["mapping_id"] = mapping.mapping_id;
    json["source_protocol"] = to_string(mapping.source_protocol);
    json["source_address"] = mapping.source_address;
    json["target_protocol"] = to_string(mapping.target_protocol);
    json["target_address"] = mapping.target_address;
    json["value_type"] = mapping.value_type;
    json["scale"] = mapping.scale;
    json["offset"] = mapping.offset;
    json["description"] = mapping.description;
    return json;
}

Json to_json(const TelemetryPoint& point) {
    Json json = Json::object();
    json["protocol"] = to_string(point.protocol);
    json["address"] = point.address;
    json["value"] = point.value;
    json["value_type"] = point.value_type;
    json["timestamp"] = point.timestamp;
    json["quality"] = point.quality;
    json["metadata"] = point.metadata;
    return json;
}

Json to_json(const ConversionResult& result) {
    Json json = Json::object();
    json["source"] = to_json(result.source);
    json["target"] = to_json(result.target);
    json["encoded_payload"] = result.encoded_payload;
    return json;
}

ChannelConfig channel_from_json(const Json& json) {
    return ChannelConfig{
        json.at("channel_id").as_string(),
        json.at("name").as_string(),
        protocol_from_string(json.at("protocol").as_string()),
        json["enabled"].as_bool(true),
        json.contains("endpoint") ? json["endpoint"] : Json::object(),
        json.contains("options") ? json["options"] : Json::object(),
    };
}

PointMapping mapping_from_json(const Json& json) {
    PointMapping mapping;
    mapping.mapping_id = json.at("mapping_id").as_string();
    mapping.source_protocol = protocol_from_string(json.at("source_protocol").as_string());
    mapping.source_address = json.at("source_address").as_string();
    mapping.target_protocol = protocol_from_string(json.at("target_protocol").as_string());
    mapping.target_address = json.at("target_address").as_string();
    mapping.value_type = json["value_type"].as_string_or("float");
    mapping.scale = json["scale"].as_number(1.0);
    mapping.offset = json["offset"].as_number(0.0);
    mapping.description = json["description"].as_string_or("");
    return mapping;
}

ConversionRequest conversion_request_from_json(const Json& json) {
    return ConversionRequest{
        protocol_from_string(json.at("source_protocol").as_string()),
        protocol_from_string(json.at("target_protocol").as_string()),
        json.at("payload"),
    };
}

double unix_time_seconds() {
    const auto now = std::chrono::system_clock::now().time_since_epoch();
    return std::chrono::duration<double>(now).count();
}

}  // namespace protocol_gateway

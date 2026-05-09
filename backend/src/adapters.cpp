#include "protocol_gateway/adapters.hpp"

#include <cstdlib>
#include <sstream>
#include <stdexcept>

namespace protocol_gateway {

namespace {

Json metadata_with(std::initializer_list<std::pair<const std::string, Json>> values) {
    Json json = Json::object();
    for (const auto& [key, value] : values) json[key] = value;
    return json;
}

std::string json_to_address(const Json& value) {
    if (value.is_string()) return value.as_string();
    if (value.is_number()) {
        std::ostringstream out;
        out << value.as_int();
        return out.str();
    }
    throw std::invalid_argument("Address field must be string or number");
}

}  // namespace

IecInformationObjectAdapter::IecInformationObjectAdapter(ProtocolType protocol, std::string display_name, int default_cot)
    : protocol_(protocol), display_name_(std::move(display_name)), default_cot_(default_cot) {}

ProtocolType IecInformationObjectAdapter::protocol() const { return protocol_; }
std::string IecInformationObjectAdapter::display_name() const { return display_name_; }

TelemetryPoint IecInformationObjectAdapter::normalize(const Json& payload) const {
    return TelemetryPoint{
        protocol_,
        json_to_address(payload.at("ioa")),
        payload.contains("value") ? payload["value"] : Json(nullptr),
        payload["type"].as_string_or("float"),
        payload["timestamp"].as_number(0.0),
        payload["quality"].as_string_or("good"),
        metadata_with({
            {"cot", payload["cot"].as_number(default_cot_)},
            {"commonAddress", payload.contains("commonAddress") ? payload["commonAddress"] : Json(1)},
        }),
    };
}

Json IecInformationObjectAdapter::encode(const TelemetryPoint& point) const {
    Json payload = Json::object();
    payload["protocol"] = to_string(protocol_);
    payload["ioa"] = point.address;
    payload["type"] = point.value_type;
    payload["value"] = point.value;
    payload["quality"] = point.quality;
    payload["timestamp"] = point.timestamp;
    payload["cot"] = point.metadata["cot"].as_number(default_cot_);
    payload["commonAddress"] = point.metadata["commonAddress"].as_number(1);
    return payload;
}

ProtocolType ModbusAdapter::protocol() const { return ProtocolType::Modbus; }
std::string ModbusAdapter::display_name() const { return "Modbus TCP/RTU"; }

TelemetryPoint ModbusAdapter::normalize(const Json& payload) const {
    const std::string function = payload["function"].as_string_or("holding_register");
    return TelemetryPoint{
        ProtocolType::Modbus,
        function + ":" + json_to_address(payload.at("address")),
        payload.contains("value") ? payload["value"] : Json(nullptr),
        payload["type"].as_string_or("int16"),
        payload["timestamp"].as_number(0.0),
        payload["quality"].as_string_or("good"),
        metadata_with({
            {"unitId", payload["unitId"].as_number(1)},
            {"function", function},
        }),
    };
}

Json ModbusAdapter::encode(const TelemetryPoint& point) const {
    const auto delimiter = point.address.find(':');
    const std::string function = delimiter == std::string::npos ? point.metadata["function"].as_string_or("holding_register") : point.address.substr(0, delimiter);
    const std::string address = delimiter == std::string::npos ? point.address : point.address.substr(delimiter + 1);

    Json payload = Json::object();
    payload["protocol"] = to_string(ProtocolType::Modbus);
    payload["unitId"] = point.metadata["unitId"].as_number(1);
    payload["function"] = function;
    payload["address"] = std::atoi(address.c_str());
    payload["type"] = point.value_type;
    payload["value"] = point.value;
    payload["quality"] = point.quality;
    payload["timestamp"] = point.timestamp;
    return payload;
}

ProtocolType Iec61850Adapter::protocol() const { return ProtocolType::Iec61850; }
std::string Iec61850Adapter::display_name() const { return "IEC 61850 MMS/GOOSE"; }

TelemetryPoint Iec61850Adapter::normalize(const Json& payload) const {
    std::string address;
    if (payload.contains("reference")) {
        address = payload.at("reference").as_string();
    } else {
        address = payload.at("ied").as_string() + "/" + payload.at("logicalNode").as_string() + "/" +
                  payload.at("dataObject").as_string() + "/" + payload.at("dataAttribute").as_string();
    }

    return TelemetryPoint{
        ProtocolType::Iec61850,
        address,
        payload.contains("value") ? payload["value"] : Json(nullptr),
        payload["type"].as_string_or("float"),
        payload["timestamp"].as_number(0.0),
        payload["quality"].as_string_or("good"),
        metadata_with({{"functionalConstraint", payload["fc"].as_string_or("MX")}}),
    };
}

Json Iec61850Adapter::encode(const TelemetryPoint& point) const {
    Json payload = Json::object();
    payload["protocol"] = to_string(ProtocolType::Iec61850);
    payload["reference"] = point.address;
    payload["fc"] = point.metadata["functionalConstraint"].as_string_or("MX");
    payload["type"] = point.value_type;
    payload["value"] = point.value;
    payload["quality"] = point.quality;
    payload["timestamp"] = point.timestamp;
    return payload;
}

}  // namespace protocol_gateway

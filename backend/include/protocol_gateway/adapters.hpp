#pragma once

#include <memory>
#include <string>

#include "protocol_gateway/models.hpp"

namespace protocol_gateway {

class ProtocolAdapter {
public:
    virtual ~ProtocolAdapter() = default;
    virtual ProtocolType protocol() const = 0;
    virtual std::string display_name() const = 0;
    virtual TelemetryPoint normalize(const Json& payload) const = 0;
    virtual Json encode(const TelemetryPoint& point) const = 0;
};

class IecInformationObjectAdapter final : public ProtocolAdapter {
public:
    IecInformationObjectAdapter(ProtocolType protocol, std::string display_name, int default_cot);
    ProtocolType protocol() const override;
    std::string display_name() const override;
    TelemetryPoint normalize(const Json& payload) const override;
    Json encode(const TelemetryPoint& point) const override;

private:
    ProtocolType protocol_;
    std::string display_name_;
    int default_cot_;
};

class ModbusAdapter final : public ProtocolAdapter {
public:
    ProtocolType protocol() const override;
    std::string display_name() const override;
    TelemetryPoint normalize(const Json& payload) const override;
    Json encode(const TelemetryPoint& point) const override;
};

class Iec61850Adapter final : public ProtocolAdapter {
public:
    ProtocolType protocol() const override;
    std::string display_name() const override;
    TelemetryPoint normalize(const Json& payload) const override;
    Json encode(const TelemetryPoint& point) const override;
};

}  // namespace protocol_gateway

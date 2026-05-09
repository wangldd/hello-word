#pragma once

#include "protocol_gateway/registry.hpp"
#include "protocol_gateway/store.hpp"

namespace protocol_gateway {

class ConversionEngine {
public:
    ConversionEngine(const ProtocolRegistry& registry, const JsonConfigStore& store);
    ConversionResult convert(const ConversionRequest& request) const;

private:
    static TelemetryPoint apply_mapping(const TelemetryPoint& source, ProtocolType target_protocol, const PointMapping& mapping);

    const ProtocolRegistry& registry_;
    const JsonConfigStore& store_;
};

}  // namespace protocol_gateway

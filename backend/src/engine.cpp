#include "protocol_gateway/engine.hpp"

namespace protocol_gateway {

ConversionEngine::ConversionEngine(const ProtocolRegistry& registry, const JsonConfigStore& store)
    : registry_(registry), store_(store) {}

ConversionResult ConversionEngine::convert(const ConversionRequest& request) const {
    const ProtocolAdapter& source_adapter = registry_.get(request.source_protocol);
    const ProtocolAdapter& target_adapter = registry_.get(request.target_protocol);
    TelemetryPoint source = source_adapter.normalize(request.payload);
    TelemetryPoint target = source;
    target.protocol = request.target_protocol;

    const auto mapping = store_.find_mapping(request.source_protocol, request.target_protocol, source.address);
    if (mapping.has_value()) target = apply_mapping(source, request.target_protocol, mapping.value());

    return ConversionResult{source, target, target_adapter.encode(target)};
}

TelemetryPoint ConversionEngine::apply_mapping(const TelemetryPoint& source, ProtocolType target_protocol, const PointMapping& mapping) {
    TelemetryPoint target = source;
    target.protocol = target_protocol;
    target.address = mapping.target_address;
    target.value_type = mapping.value_type;
    if (target.value.is_number()) {
        target.value = target.value.as_number() * mapping.scale + mapping.offset;
    }
    return target;
}

}  // namespace protocol_gateway

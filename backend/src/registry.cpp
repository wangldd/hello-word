#include "protocol_gateway/registry.hpp"

#include <stdexcept>

namespace protocol_gateway {

void ProtocolRegistry::register_adapter(std::unique_ptr<ProtocolAdapter> adapter) {
    adapters_[adapter->protocol()] = std::move(adapter);
}

const ProtocolAdapter& ProtocolRegistry::get(ProtocolType protocol) const {
    auto it = adapters_.find(protocol);
    if (it == adapters_.end()) throw std::invalid_argument("Unsupported protocol: " + to_string(protocol));
    return *it->second;
}

Json ProtocolRegistry::list_protocols() const {
    Json::Array protocols;
    for (const auto& [protocol, adapter] : adapters_) {
        Json item = Json::object();
        item["value"] = to_string(protocol);
        item["label"] = adapter->display_name();
        protocols.push_back(item);
    }
    return protocols;
}

ProtocolRegistry create_default_registry() {
    ProtocolRegistry registry;
    registry.register_adapter(std::make_unique<IecInformationObjectAdapter>(ProtocolType::Iec101, "IEC 60870-5-101", 3));
    registry.register_adapter(std::make_unique<IecInformationObjectAdapter>(ProtocolType::Iec102, "IEC 60870-5-102", 3));
    registry.register_adapter(std::make_unique<IecInformationObjectAdapter>(ProtocolType::Iec103, "IEC 60870-5-103", 3));
    registry.register_adapter(std::make_unique<IecInformationObjectAdapter>(ProtocolType::Iec104, "IEC 60870-5-104", 3));
    registry.register_adapter(std::make_unique<ModbusAdapter>());
    registry.register_adapter(std::make_unique<Iec61850Adapter>());
    return registry;
}

}  // namespace protocol_gateway

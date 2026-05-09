#pragma once

#include <map>
#include <memory>
#include <vector>

#include "protocol_gateway/adapters.hpp"

namespace protocol_gateway {

class ProtocolRegistry {
public:
    void register_adapter(std::unique_ptr<ProtocolAdapter> adapter);
    const ProtocolAdapter& get(ProtocolType protocol) const;
    Json list_protocols() const;

private:
    std::map<ProtocolType, std::unique_ptr<ProtocolAdapter>> adapters_;
};

ProtocolRegistry create_default_registry();

}  // namespace protocol_gateway

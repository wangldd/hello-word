#pragma once

#include <optional>
#include <string>
#include <vector>

#include "protocol_gateway/models.hpp"

namespace protocol_gateway {

class JsonConfigStore {
public:
    explicit JsonConfigStore(std::string path);

    std::vector<ChannelConfig> list_channels() const;
    ChannelConfig save_channel(const ChannelConfig& channel);

    std::vector<PointMapping> list_mappings() const;
    PointMapping save_mapping(const PointMapping& mapping);
    std::optional<PointMapping> find_mapping(
        ProtocolType source_protocol,
        ProtocolType target_protocol,
        const std::string& source_address) const;

private:
    Json read() const;
    void write(const Json& data) const;
    void ensure_file() const;

    std::string path_;
};

}  // namespace protocol_gateway

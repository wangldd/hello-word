#include "protocol_gateway/store.hpp"

#include <filesystem>
#include <fstream>

namespace protocol_gateway {

JsonConfigStore::JsonConfigStore(std::string path) : path_(std::move(path)) {
    ensure_file();
}

std::vector<ChannelConfig> JsonConfigStore::list_channels() const {
    const Json data = read();
    std::vector<ChannelConfig> channels;
    for (const auto& item : data["channels"].as_array()) channels.push_back(channel_from_json(item));
    return channels;
}

ChannelConfig JsonConfigStore::save_channel(const ChannelConfig& channel) {
    Json data = read();
    Json::Array channels;
    for (const auto& item : data["channels"].as_array()) {
        if (item["channel_id"].as_string_or("") != channel.channel_id) channels.push_back(item);
    }
    channels.push_back(to_json(channel));
    data["channels"] = channels;
    write(data);
    return channel;
}

std::vector<PointMapping> JsonConfigStore::list_mappings() const {
    const Json data = read();
    std::vector<PointMapping> mappings;
    for (const auto& item : data["mappings"].as_array()) mappings.push_back(mapping_from_json(item));
    return mappings;
}

PointMapping JsonConfigStore::save_mapping(const PointMapping& mapping) {
    Json data = read();
    Json::Array mappings;
    for (const auto& item : data["mappings"].as_array()) {
        if (item["mapping_id"].as_string_or("") != mapping.mapping_id) mappings.push_back(item);
    }
    mappings.push_back(to_json(mapping));
    data["mappings"] = mappings;
    write(data);
    return mapping;
}

std::optional<PointMapping> JsonConfigStore::find_mapping(
    ProtocolType source_protocol,
    ProtocolType target_protocol,
    const std::string& source_address) const {
    for (const auto& mapping : list_mappings()) {
        if (mapping.source_protocol == source_protocol && mapping.target_protocol == target_protocol &&
            mapping.source_address == source_address) {
            return mapping;
        }
    }
    return std::nullopt;
}

Json JsonConfigStore::read() const {
    std::ifstream input(path_);
    std::string content((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
    if (content.empty()) return Json::parse(R"({"channels":[],"mappings":[]})");
    return Json::parse(content);
}

void JsonConfigStore::write(const Json& data) const {
    std::ofstream output(path_, std::ios::trunc);
    output << data.dump(2);
}

void JsonConfigStore::ensure_file() const {
    const std::filesystem::path path(path_);
    if (path.has_parent_path()) std::filesystem::create_directories(path.parent_path());
    if (!std::filesystem::exists(path) || std::filesystem::file_size(path) == 0) {
        write(Json::parse(R"({"channels":[],"mappings":[]})"));
    }
}

}  // namespace protocol_gateway

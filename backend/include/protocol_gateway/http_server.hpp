#pragma once

#include <string>

namespace protocol_gateway {

class HttpServer {
public:
    HttpServer(std::string host, int port, std::string config_path);
    void run();

private:
    std::string handle_request(const std::string& method, const std::string& path, const std::string& body, int& status_code);
    std::string host_;
    int port_;
    std::string config_path_;
};

}  // namespace protocol_gateway

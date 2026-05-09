#include <cstdlib>
#include <iostream>
#include <string>

#include "protocol_gateway/http_server.hpp"

int main(int argc, char** argv) {
    std::string host = "0.0.0.0";
    int port = 8080;
    std::string config = "data/gateway-config.json";

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            std::cout << "Usage: protocol_gateway_server [--host HOST] [--port PORT] [--config PATH]\n";
            return 0;
        }
        if (arg == "--host" && i + 1 < argc) host = argv[++i];
        else if (arg == "--port" && i + 1 < argc) port = std::atoi(argv[++i]);
        else if (arg == "--config" && i + 1 < argc) config = argv[++i];
    }

    try {
        protocol_gateway::HttpServer(host, port, config).run();
    } catch (const std::exception& ex) {
        std::cerr << "Failed to start protocol gateway: " << ex.what() << '\n';
        return 1;
    }
    return 0;
}

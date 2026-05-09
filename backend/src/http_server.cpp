#include "protocol_gateway/http_server.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <sstream>
#include <stdexcept>

#include "protocol_gateway/engine.hpp"

namespace protocol_gateway {

namespace {

std::string reason_phrase(int status) {
    switch (status) {
        case 200: return "OK";
        case 201: return "Created";
        case 204: return "No Content";
        case 400: return "Bad Request";
        case 404: return "Not Found";
        default: return "Internal Server Error";
    }
}

std::string http_response(int status, const std::string& body) {
    std::ostringstream response;
    response << "HTTP/1.1 " << status << ' ' << reason_phrase(status) << "\r\n"
             << "Content-Type: application/json; charset=utf-8\r\n"
             << "Access-Control-Allow-Origin: *\r\n"
             << "Access-Control-Allow-Methods: GET,POST,OPTIONS\r\n"
             << "Access-Control-Allow-Headers: Content-Type\r\n"
             << "Content-Length: " << body.size() << "\r\n"
             << "Connection: close\r\n\r\n"
             << body;
    return response.str();
}

std::string json_error(const std::string& message) {
    Json body = Json::object();
    body["error"] = message;
    return body.dump();
}

}  // namespace

HttpServer::HttpServer(std::string host, int port, std::string config_path)
    : host_(std::move(host)), port_(port), config_path_(std::move(config_path)) {}

void HttpServer::run() {
    int server_fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) throw std::runtime_error("socket() failed");

    int reuse = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(static_cast<uint16_t>(port_));
    if (::inet_pton(AF_INET, host_.c_str(), &address.sin_addr) != 1) {
        address.sin_addr.s_addr = INADDR_ANY;
    }

    if (::bind(server_fd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0) {
        ::close(server_fd);
        throw std::runtime_error("bind() failed");
    }
    if (::listen(server_fd, 16) < 0) {
        ::close(server_fd);
        throw std::runtime_error("listen() failed");
    }

    std::cout << "Protocol gateway C++ API listening on http://" << host_ << ':' << port_ << std::endl;
    while (true) {
        int client_fd = ::accept(server_fd, nullptr, nullptr);
        if (client_fd < 0) continue;

        char buffer[65536];
        const ssize_t received = ::recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        if (received <= 0) {
            ::close(client_fd);
            continue;
        }
        buffer[received] = '\0';
        std::string request(buffer);
        const auto header_end = request.find("\r\n\r\n");
        const std::string header = request.substr(0, header_end);
        const std::string body = header_end == std::string::npos ? "" : request.substr(header_end + 4);

        std::istringstream lines(header);
        std::string method;
        std::string path;
        std::string version;
        lines >> method >> path >> version;
        const auto query = path.find('?');
        if (query != std::string::npos) path = path.substr(0, query);

        int status = 200;
        std::string response_body;
        try {
            response_body = handle_request(method, path, body, status);
        } catch (const std::exception& ex) {
            status = 400;
            response_body = json_error(ex.what());
        }

        const std::string response = http_response(status, response_body);
        ::send(client_fd, response.data(), response.size(), 0);
        ::close(client_fd);
    }
}

std::string HttpServer::handle_request(const std::string& method, const std::string& path, const std::string& body, int& status_code) {
    ProtocolRegistry registry = create_default_registry();
    JsonConfigStore store(config_path_);
    ConversionEngine engine(registry, store);

    if (method == "OPTIONS") {
        status_code = 200;
        return R"({"ok":true})";
    }
    if (method == "GET" && path == "/api/health") {
        status_code = 200;
        return R"({"status":"ok"})";
    }
    if (method == "GET" && path == "/api/protocols") {
        status_code = 200;
        return registry.list_protocols().dump();
    }
    if (method == "GET" && path == "/api/channels") {
        Json::Array channels;
        for (const auto& channel : store.list_channels()) channels.push_back(to_json(channel));
        status_code = 200;
        return Json(channels).dump();
    }
    if (method == "GET" && path == "/api/mappings") {
        Json::Array mappings;
        for (const auto& mapping : store.list_mappings()) mappings.push_back(to_json(mapping));
        status_code = 200;
        return Json(mappings).dump();
    }
    if (method == "POST" && path == "/api/channels") {
        status_code = 201;
        return to_json(store.save_channel(channel_from_json(Json::parse(body)))).dump();
    }
    if (method == "POST" && path == "/api/mappings") {
        status_code = 201;
        return to_json(store.save_mapping(mapping_from_json(Json::parse(body)))).dump();
    }
    if (method == "POST" && path == "/api/convert") {
        status_code = 200;
        return to_json(engine.convert(conversion_request_from_json(Json::parse(body)))).dump();
    }

    status_code = 404;
    return json_error("not found");
}

}  // namespace protocol_gateway

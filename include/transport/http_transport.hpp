#pragma once
#include "transport.hpp"
#include "type/mcp_type.hpp"
#include <httplib.h>

namespace mcp {

class HttpTransport : public Transport {
    std::string host;
    std::string path;

    type::HttpConfig config;

public:
    HttpTransport(const type::HttpConfig& config)
        : config(config) {}

    void send(const std::string& message) override {
        httplib::Client cli(config.baseUrl);
        cli.Post(config.baseUrl.c_str(), message, "application/json");
    }

    void start(MessageHandler) override {
        // HTTP is stateless, so nothing to "listen" to
    }

    void stop() override {}
};

} // namespace mcp
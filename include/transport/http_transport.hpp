#pragma once
#include "transport.hpp"
#include <httplib.h>

namespace mcp {

class HttpTransport : public Transport {
    std::string host;
    std::string path;

public:
    HttpTransport(const std::string& host, const std::string& path)
        : host(host), path(path) {}

    void send(const std::string& message) override {
        httplib::Client cli(host);
        cli.Post(path.c_str(), message, "application/json");
    }

    void start(MessageHandler) override {
        // HTTP is stateless, so nothing to "listen" to
    }

    void stop() override {}
};

} // namespace mcp
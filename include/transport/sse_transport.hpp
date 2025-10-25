#pragma once
#include "transport.hpp"
#include "type/mcp_type.hpp"
#include <httplib.h>
#include <thread>
#include <atomic>

namespace mcp {

class SseTransport : public Transport {
    std::string host;
    std::string path;
    std::atomic<bool> running{false};
    std::thread listener;

    type::SseConfig config;

public:
    SseTransport(const type::SseConfig& config)
        : config(config) {}

    void send(const std::string& message) override {
        httplib::Client cli(config.url);
        cli.Post(config.messageEndpoint.c_str(), message, "application/json");
    }

    void start(MessageHandler onMessage) override {
        running = true;
        listener = std::thread([this, onMessage]() {
            httplib::Client cli(config.url);
            cli.Get(config.sseEndpoint.c_str(),
                [&](const char* data, size_t len) {
                    onMessage(std::string(data, len));
                    return true;
                }
            );
        });
    }

    void stop() override {
        running = false;
        if (listener.joinable()) listener.join();
    }
};

} // namespace mcp
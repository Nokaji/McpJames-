#pragma once
#include "transport.hpp"
#include <httplib.h>
#include <thread>
#include <atomic>

namespace mcp {

class SseTransport : public Transport {
    std::string host;
    std::string path;
    std::atomic<bool> running{false};
    std::thread listener;

public:
    SseTransport(const std::string& host, const std::string& path)
        : host(host), path(path) {}

    void send(const std::string& message) override {
        httplib::Client cli(host);
        cli.Post(path.c_str(), message, "application/json");
    }

    void start(MessageHandler onMessage) override {
        running = true;
        listener = std::thread([this, onMessage]() {
            httplib::Client cli(host);
            cli.Get(path.c_str(),
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
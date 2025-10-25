#pragma once
#include <string>
#include <functional>

namespace mcp {

class Transport {
public:
    using MessageHandler = std::function<void(const std::string&)>;

    virtual ~Transport() = default;

    virtual void send(const std::string& message) = 0;
    virtual void start(MessageHandler onMessage) = 0;
    virtual void stop() = 0;
};

}
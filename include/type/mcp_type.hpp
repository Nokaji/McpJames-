#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <map>
#include <variant>
#include <future>
#include <chrono>
#include <optional>
#include "../transport/transport.hpp"
#include <nlohmann/json.hpp>
namespace mcp {
namespace type {

struct HttpConfig {
    std::string baseUrl;
    std::map<std::string, std::string> headers;
    int timeoutMs = 30000;
    bool verifySSL = true;
};

struct WebSocketConfig {
    std::string url;
    std::map<std::string, std::string> headers;
    int timeoutMs = 30000;
    bool verifySSL = true;
};

struct SseConfig {
    std::string url;
    std::string sseEndpoint = "/sse";
    std::string messageEndpoint = "/message";
    std::map<std::string, std::string> headers;
    int timeoutMs = 30000;
    bool verifySSL = true;
    int reconnectDelayMs = 3000;
    int maxRetries = -1;
    std::string lastEventId;
};

enum class ConnectionStatus {
    DISCONNECTED,
    CONNECTING,
    CONNECTED,
    ERROR,
    RECONNECTING
};

struct McpMessage {
    std::string id;
    std::string method;
    std::map<std::string, std::string> params;
    std::string result;
    std::string error;
    bool isRequest = true;
};

struct McpServerConfig {
    std::string name;
    std::string description;
    bool autoReconnect = true;
    int maxRetries = 3;
    int retryDelayMs = 1000;
};

using ConnectionCallback = std::function<void(const std::string& serverId, ConnectionStatus status)>;
using MessageCallback = std::function<void(const std::string& serverId, const McpMessage& message)>;
using ErrorCallback = std::function<void(const std::string& serverId, const std::string& error)>;

class IMcpTransport {
public:
    virtual ~IMcpTransport() = default;
    virtual bool connect() = 0;
    virtual void disconnect() = 0;
    virtual bool isConnected() const = 0;
    virtual std::future<std::string> sendMessage(const McpMessage& message) = 0;
    virtual void setMessageCallback(MessageCallback callback) = 0;
    virtual void setErrorCallback(ErrorCallback callback) = 0;
};

struct McpServerInfo {
    std::string id;
    McpServerConfig config;
    std::unique_ptr<Transport> transport;
    ConnectionStatus status = ConnectionStatus::DISCONNECTED;
    std::chrono::steady_clock::time_point lastConnected;
    int retryCount = 0;
};
}
};
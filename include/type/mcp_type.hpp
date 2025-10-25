/*
 * Copyright (c) 2025 Nokaji. Tous droits réservés.
 * Ce fichier fait partie du projet Kernel-James.
 */
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
#include "transport/transport.hpp"
#include <nlohmann/json.hpp>
namespace mcp {
namespace type {

// Configuration for HTTP transport
struct HttpConfig {
    std::string baseUrl;
    std::map<std::string, std::string> headers;
    int timeoutMs = 30000;
    bool verifySSL = true;
};

// Configuration for WebSocket transport
struct WebSocketConfig {
    std::string url;
    std::map<std::string, std::string> headers;
    int timeoutMs = 30000;
    bool verifySSL = true;
};

// Configuration for SSE (Server-Sent Events) transport
struct SseConfig {
    std::string url;              // Base URL (e.g., "http://localhost:8080")
    std::string sseEndpoint = "/sse";      // SSE endpoint for receiving events
    std::string messageEndpoint = "/message"; // HTTP endpoint for sending messages
    std::map<std::string, std::string> headers;
    int timeoutMs = 30000;
    bool verifySSL = true;
    int reconnectDelayMs = 3000;
    std::string lastEventId;
};

// MCP connection status
enum class ConnectionStatus {
    DISCONNECTED,
    CONNECTING,
    CONNECTED,
    ERROR,
    RECONNECTING
};

// MCP message
struct McpMessage {
    std::string id;
    std::string method;
    std::map<std::string, std::string> params;
    std::string result;
    std::string error;
    bool isRequest = true;
};

// MCP server configuration
struct McpServerConfig {
    std::string name;
    std::string description;
    bool autoReconnect = true;
    int maxRetries = 3;
    int retryDelayMs = 1000;

    NLOHMANN_DEFINE_DERIVED_TYPE_INTRUSIVE(McpServerConfig, name, description, autoReconnect, maxRetries, retryDelayMs);
};

// Callbacks for MCP events
using ConnectionCallback = std::function<void(const std::string& serverId, ConnectionStatus status)>;
using MessageCallback = std::function<void(const std::string& serverId, const McpMessage& message)>;
using ErrorCallback = std::function<void(const std::string& serverId, const std::string& error)>;

// MCP transport interface
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

// Information about a connected MCP server
struct McpServerInfo {
    std::string id;
    McpServerConfig config;
    std::unique_ptr<Transport> transport;
    ConnectionStatus status = ConnectionStatus::DISCONNECTED;
    std::chrono::steady_clock::time_point lastConnected;
    int retryCount = 0;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(McpServerInfo, id, config, status, lastConnected, retryCount);
};
}
}; // namespace mcp::type
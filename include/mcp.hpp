#pragma once
#include "transport/transport.hpp"
#include "type/mcp_type.hpp"
#include "jsonrpc.hpp"
#include <memory>
#include <chrono>

namespace mcp {

class mcp {
    std::string id;
    type::McpServerConfig config;
    std::unique_ptr<Transport> transport;
    type::ConnectionStatus status = type::ConnectionStatus::DISCONNECTED;
    std::chrono::steady_clock::time_point lastConnected;
    int retryCount = 0;
    
    int nextId = 1;

public:
    explicit mcp(std::unique_ptr<Transport> t)
        : transport(std::move(t)) {}

    void start() {
        transport->start([this](const std::string& msg) {
            auto res = JsonRpc::parseResponse(msg);
            // gérer les callbacks si besoin
        });
    }

    nlohmann::json call(const std::string& method, const nlohmann::json& params) {
        JsonRpcRequest req{ "2.0", nextId++, method, params };
        auto msg = JsonRpc::serializeRequest(req);
        transport->send(msg);
        // gestion simplifiée : ici tu pourrais attendre la réponse
        return {};
    }

    void stop() { transport->stop(); }
};

} // namespace mcp
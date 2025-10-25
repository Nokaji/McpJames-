#pragma once
#include "transport/transport.hpp"
#include "type/mcp_type.hpp"
#include "jsonrpc.hpp"
#include <memory>
#include <chrono>
#include <iostream>
#include <regex>
#include <mutex>

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
        std::cout << "[MCP] Starting transport and listening for responses..." << std::endl;
        transport->start([this](const std::string& msg) {
            std::cout << "\n[MCP] <<<< Received raw message: " << msg << std::endl;
            
            try {
                auto res = JsonRpc::parseResponse(msg);
                std::cout << "[MCP] Parsed response:" << std::endl;
                std::cout << "  - ID: " << res.id << std::endl;
                if (!res.error.is_null()) {
                    std::cout << "  - Error: " << res.error.dump(2) << std::endl;
                } else {
                    std::cout << "  - Result: " << res.result.dump(2) << std::endl;
                }
            } catch (const std::exception& e) {
                std::cout << "[MCP] Error parsing response: " << e.what() << std::endl;
            }
        });
    }

    nlohmann::json call(const std::string& method, const nlohmann::json& params) {
        JsonRpcRequest req{ "2.0", nextId++, method, params };
        auto msg = JsonRpc::serializeRequest(req);
        
        std::cout << "[MCP] >>>> Sending request (id=" << req.id << "):" << std::endl;
        std::cout << "  - Method: " << method << std::endl;
        std::cout << "  - Params: " << params.dump(2) << std::endl;
        std::cout << "  - Raw JSON: " << msg << std::endl;
        
        transport->send(msg);
        return {};
    }

    void stop() { 
        std::cout << "[MCP] Stopping transport..." << std::endl;
        transport->stop(); 
    }
    

};

}
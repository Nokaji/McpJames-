#include "mcp.hpp"

namespace mcp {
    Mcp::Mcp(std::shared_ptr<types::McpServerInfo> config) {
        // Initialize serverInfo from config
        serverInfo.config = config->config;
        serverInfo.transport = std::move(config->transport);
        serverInfo.id = config->id;
        serverInfo.status = types::ConnectionStatus::DISCONNECTED;

        // Start MCP thread (placeholder logic)
    }

    Mcp::~Mcp() {
        if (mcpThread.joinable()) {
            mcpThread.join();
        }
    }
}
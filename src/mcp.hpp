#pragma once

#include <thread>
#include "types/mcp_types.h"

namespace mcp {
    class Mcp {
    private:
        types::McpServerInfo serverInfo;
    public:
        Mcp(std::shared_ptr<types::McpServerInfo> config);
        ~Mcp();
    };
}
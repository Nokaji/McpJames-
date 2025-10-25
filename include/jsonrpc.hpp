#pragma once
#include <string>
#include <nlohmann/json.hpp>

namespace mcp {

struct JsonRpcRequest {
    std::string jsonrpc = "2.0";
    int id;
    std::string method;
    nlohmann::json params;
};

struct JsonRpcResponse {
    std::string jsonrpc = "2.0";
    int id;
    nlohmann::json result;
    nlohmann::json error;
};

class JsonRpc {
public:
    static std::string serializeRequest(const JsonRpcRequest& req);
    static std::string serializeResponse(const JsonRpcResponse& res);
    static JsonRpcRequest parseRequest(const std::string& data);
    static JsonRpcResponse parseResponse(const std::string& data);
};

} // namespace mcp
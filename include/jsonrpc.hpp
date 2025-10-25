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
    static inline std::string serializeRequest(const JsonRpcRequest& req) {
        nlohmann::json j;
        j["jsonrpc"] = req.jsonrpc;
        j["id"] = req.id;
        j["method"] = req.method;
        
        if (!req.params.is_null()) {
            j["params"] = req.params;
        }
        
        return j.dump();
    }

    static inline std::string serializeResponse(const JsonRpcResponse& res) {
        nlohmann::json j;
        j["jsonrpc"] = res.jsonrpc;
        j["id"] = res.id;
        if (!res.error.is_null()) {
            j["error"] = res.error;
        } else {
            j["result"] = res.result;
        }
        return j.dump();
    }

    static inline JsonRpcRequest parseRequest(const std::string& data) {
        auto j = nlohmann::json::parse(data);
        JsonRpcRequest req;
        req.jsonrpc = j.value("jsonrpc", "2.0");
        req.id = j.value("id", 0);
        req.method = j.value("method", "");
        req.params = j.value("params", nlohmann::json::object());
        return req;
    }

    static inline JsonRpcResponse parseResponse(const std::string& data) {
        auto j = nlohmann::json::parse(data);
        JsonRpcResponse res;
        res.jsonrpc = j.value("jsonrpc", "2.0");
        res.id = j.value("id", 0);
        
        if (j.contains("error")) {
            res.error = j["error"];
        } else if (j.contains("result")) {
            res.result = j["result"];
        }
        
        return res;
    }
};

}

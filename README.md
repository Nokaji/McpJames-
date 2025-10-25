# McpJames_plusplus — kernel "james"

McpJames_plusplus provides the "james" kernel: a small C++/CMake executable intended to operate within a Model Context Protocol (MCP) ecosystem.

Designed to simplify the management of MCP servers with Supergateway and other gateway solutions.

## Based on

- [Model Context Protocol — Architecture](https://modelcontextprotocol.io/docs/learn/architecture)

## Uses

- [Supergateway (supercorp-ai)](https://github.com/supercorp-ai/supergateway)

## Overview

- Kernel "james": a C++ executable in this repository (built with CMake).
- Purpose: act as a backend kernel that a gateway (for example Supergateway) can start and interact with according to MCP conventions.

## Contribute

Contributions are welcome (issues, PRs). For larger changes, please open an issue first to discuss the approach.

## Resources

- [MCP docs — Architecture](https://modelcontextprotocol.io/docs/learn/architecture)
- [Supergateway repo](https://github.com/supercorp-ai/supergateway)

## Exemple d'utilisation (SSE)

```cpp
	std::cout << "=== Test function started ===" << std::endl;

	mcp::type::SseConfig sseConfig;
	sseConfig.url = "http://localhost:8080";
	sseConfig.sseEndpoint = "/sse";
	sseConfig.messageEndpoint = "/message";

	std::cout << "Creating MCP client with:" << std::endl;
	std::cout << "  URL: " << sseConfig.url << std::endl;
	std::cout << "  SSE Endpoint: " << sseConfig.sseEndpoint << std::endl;
	std::cout << "  Message Endpoint: " << sseConfig.messageEndpoint << std::endl;

	mcp::mcp websearch(std::make_unique<mcp::SseTransport>(sseConfig));

	std::cout << "Starting MCP transport..." << std::endl;
	websearch.start();

	std::cout << "Waiting for SSE connection to establish..." << std::endl;

	std::cout << "\n--- Calling initialize ---" << std::endl;
	nlohmann::json initParams = nlohmann::json::object();
	initParams["protocolVersion"] = "2024-11-05";
	initParams["capabilities"] = nlohmann::json::object();
	initParams["clientInfo"] = {
		{"name", "Kernel-JAMES"},
		{"version", "1.0.0"}
	};
	websearch.call("initialize", initParams);

	std::cout << "Waiting for initialize response..." << std::endl;

	std::cout << "\n--- Calling tools/list ---" << std::endl;
	websearch.call("tools/list", nlohmann::json::object());

	std::cout << "Waiting for tools/list response..." << std::endl;

	std::cout << "\nStopping MCP transport..." << std::endl;
	websearch.stop();

	std::cout << "=== Test function completed ===" << std::endl;
```

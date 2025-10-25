set(MCPJAMESPLUSPLUS_HEADERS
    include/mcp.hpp
    include/jsonrpc.hpp
    include/type/mcp_type.hpp
    include/transport/transport.hpp
    include/transport/http_transport.hpp
    include/transport/sse_transport.hpp
)

# Si aucune source .cpp n'est fournie, on déclare une lib header-only en INTERFACE
add_library(mcpjamesplusplus INTERFACE)

find_package(CURL REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(httplib CONFIG REQUIRED)

target_link_libraries(mcpjamesplusplus
    INTERFACE
    CURL::libcurl
    fmt::fmt
    nlohmann_json::nlohmann_json
    httplib::httplib
)

target_include_directories(mcpjamesplusplus
    INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Spécifique Windows
if (WIN32)
    target_link_libraries(mcpjamesplusplus PRIVATE ws2_32)
endif ()
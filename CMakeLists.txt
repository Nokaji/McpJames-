set(MCPJAMESPLUSPLUS_HEADERS
    include/mcp.hpp
    include/jsonrpc.hpp
    include/type/mcp_type.hpp
    include/transport/transport.hpp
    include/transport/http_transport.hpp
    include/transport/sse_transport.hpp
)

add_library(mcpjamesplusplus STATIC ${MCPJAMESPLUSPLUS_SOURCES} ${MCPJAMESPLUSPLUS_HEADERS})

find_package(CURL REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)

target_link_libraries(mcpjamesplusplus
)

target_include_directories(mcpjamesplusplus
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
        PUBLIC ${CMAKE_SOURCE_DIR}
)

# Spécifique Windows
if (WIN32)
    target_link_libraries(mcpjamesplusplus PRIVATE ws2_32)
endif ()
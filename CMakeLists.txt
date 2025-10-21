set(MCPJAMESPLUSPLUS_SOURCES
    src/base/mcp.cpp
    src/mcpSSE.cpp
    src/transport/SSE.cpp
)

set(MCPJAMESPLUSPLUS_HEADERS
    src/base/mcp.hpp
    src/mcpSSE.h
    src/types/mcp_types.h
    src/transport/SSE.hpp
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
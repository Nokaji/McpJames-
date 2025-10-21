#include "SSE.hpp"
#include <httplib.h>

SSE::SSE() {
    // Start SSE thread (placeholder logic)
    sseThread = std::thread([this]() {
        
    });
}

SSE::~SSE() {
    if (sseThread.joinable()) {
        sseThread.join();
    }
}
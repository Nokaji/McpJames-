#pragma once

#include <thread>
#include <httplib.h>
#include <iostream>

class SSE {
private:
    std::thread sseThread;

public:
    SSE();
    ~SSE();
};
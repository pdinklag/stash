#pragma once

#include <chrono>

// time in ms (milliseconds)
inline uint64_t time() {
    using namespace std::chrono;
    
    return uint64_t(duration_cast<milliseconds>(
        system_clock::now().time_since_epoch()).count());
}

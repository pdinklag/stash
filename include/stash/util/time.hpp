#pragma once

#include <chrono>

namespace stash {

// time in ms (milliseconds)
inline uint64_t time() {
    using namespace std::chrono;
    
    return uint64_t(duration_cast<milliseconds>(
        system_clock::now().time_since_epoch()).count());
}

// time in ns (nanoseconds)
inline uint64_t time_nanos() {
    using namespace std::chrono;
    
    return uint64_t(duration_cast<nanoseconds>(
        system_clock::now().time_since_epoch()).count());
}

}

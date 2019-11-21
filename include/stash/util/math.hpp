#pragma once

#include <cstdint>
#include <stash/util/likely.hpp>

namespace stash {

inline constexpr uint64_t idiv_ceil(uint64_t a, uint64_t b) {
    const uint64_t q = a / b;
    return (a % b == 0) ? q : q + 1;
}

inline constexpr uint64_t log2_ceil(uint64_t x) {
    return 64ULL - __builtin_clzll(x);
}

inline constexpr uint64_t log2_floor(uint64_t x) {
    if(unlikely(x == 0)) return 0;
    return 64ULL - __builtin_clzll(x) - 1ULL;
}

}
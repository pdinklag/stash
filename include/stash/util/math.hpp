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

// tests whether p is a prime
// TODO: use wheel factorization, this is too slow
inline constexpr bool is_prime(uint64_t p) {
    if(p % 2 == 0) return false;
    
    const uint64_t m = p >> 1;
    for(size_t i = 2; i <= m; i ++) {
        if((p % i) == 0) {
            return false;
        }
    }
    return true;
}

// finds the smallest prime greater than or equal to p
inline uint64_t prime_successor(uint64_t p) {
    if(unlikely(p == 0)) return 0;
    if(unlikely(p == 2)) return 2;
    if(p % 2 == 0) ++p; // all primes > 2 are odd

    // linear search - the gap between two primes is hopefully very low
    // in the worst case, because there must be a prime between p and 2p, this takes p steps
    while(!is_prime(p)) p += 2;
    return p;
}

}

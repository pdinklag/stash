#pragma once

#include <cstdint>
#include <stash/util/likely.hpp>

namespace stash {

// the first 94 fibonacci numbers, all of which fit into 64 bits
constexpr uint64_t fib[] = {
    0ULL,1ULL,1ULL,2ULL,3ULL,5ULL,8ULL,13ULL,21ULL,34ULL,55ULL,89ULL,144ULL,233ULL,
    377ULL,610ULL,987ULL,1597ULL,2584ULL,4181ULL,6765ULL,10946ULL,17711ULL,28657ULL,
    46368ULL,75025ULL,121393ULL,196418ULL,317811ULL,514229ULL,832040ULL,1346269ULL,
    2178309ULL,3524578ULL,5702887ULL,9227465ULL,14930352ULL,24157817ULL,39088169ULL,
    63245986ULL,102334155ULL,165580141ULL,267914296ULL,433494437ULL,701408733ULL,
    1134903170ULL,1836311903ULL,2971215073ULL,4807526976ULL,7778742049ULL,
    12586269025ULL,20365011074ULL,32951280099ULL,53316291173ULL,86267571272ULL,
    139583862445ULL,225851433717ULL,365435296162ULL,591286729879ULL,956722026041ULL,
    1548008755920ULL,2504730781961ULL,4052739537881ULL,6557470319842ULL,
    10610209857723ULL,17167680177565ULL,27777890035288ULL,44945570212853ULL,
    72723460248141ULL,117669030460994ULL,190392490709135ULL,308061521170129ULL,
    498454011879264ULL,806515533049393ULL,1304969544928657ULL,2111485077978050ULL,
    3416454622906707ULL,5527939700884757ULL,8944394323791464ULL,
    14472334024676221ULL,23416728348467685ULL,37889062373143906ULL,
    61305790721611591ULL,99194853094755497ULL,160500643816367088ULL,
    259695496911122585ULL,420196140727489673ULL,679891637638612258ULL,
    1100087778366101931ULL,1779979416004714189ULL,2880067194370816120ULL,
    4660046610375530309ULL,7540113804746346429ULL,12200160415121876738ULL
};

inline constexpr uint64_t idiv_ceil(const uint64_t a, const uint64_t b) {
    const uint64_t q = a / b;
    return (a % b == 0) ? q : q + 1;
}

inline constexpr uint64_t log2_ceil(const uint64_t x) {
    return 64ULL - __builtin_clzll(x);
}

inline constexpr uint64_t log2_floor(const uint64_t x) {
    if(unlikely(x == 0)) return 0;
    return 64ULL - __builtin_clzll(x) - 1ULL;
}

inline constexpr uint64_t isqrt_floor(const uint64_t x) {
    if(unlikely(x < 4ULL)) return uint64_t(x > 0);
    
    uint64_t e = log2_floor(x) >> 1;
    uint64_t r = 1ULL;

    while(e--) {
        const uint64_t cur = x >> (e << 1ULL);
        const uint64_t sm = r << 1ULL;
        const uint64_t lg = sm + 1ULL;
        r = (sm + (lg * lg <= cur));
    }
    return r;
}

inline constexpr uint64_t isqrt_ceil(const uint64_t x) {
    const uint64_t r = isqrt_floor(x);
    return r + (r * r < x);
}

// tests whether p is a prime
// TODO: use wheel factorization, this is too slow!
inline constexpr bool is_prime(const uint64_t p) {
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

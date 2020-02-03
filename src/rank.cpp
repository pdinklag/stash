#include <cstdint>
#include <iostream>
#include <random>

#include <stash/rapl/reader.hpp>
#include <stash/rapl/power.hpp>
#include <stash/util/rank8_lut.hpp>
#include <stash/util/rank16_lut.hpp>
#include <stash/util/time.hpp>

#include <emmintrin.h>

#include <tlx/cmdline_parser.hpp>

using namespace stash;

// shift
inline uint64_t rank_shift(const std::vector<uint64_t>& a) {
    uint64_t rank = 0;
    const size_t n = a.size();
    for(size_t i = 0; i < n; i++) {
        uint64_t x = a[i];
        while(x) {
            rank += (x & 1);
            x >>= 1;
        }
    }
    return rank;
}

// methods by Sean Anderson (found on Bit Twiddling Hacks)
inline uint64_t rank14_anderson(const uint64_t x) {
    return (x * 0x200040008001ULL & 0x111111111111111ULL) % 0xf;
}

inline uint64_t rank_anderson14(const std::vector<uint64_t>& a) {
    uint64_t rank = 0;
    const size_t n = a.size();
    for(size_t i = 0; i < n; i++) {
        uint64_t x = a[i];
        rank += rank14_anderson(x & 16383ULL) +
                rank14_anderson((x >> 14ULL) & 16383ULL) +
                rank14_anderson((x >> 28ULL) & 16383ULL) +
                rank14_anderson((x >> 42ULL) & 16383ULL) +
                rank14_anderson(x >> 56ULL);
    }
    return rank;
}

inline uint64_t rank24_anderson(const uint64_t x) {
    return
        ((x & 0xfff) * 0x1001001001001ULL & 0x84210842108421ULL) % 0x1f +
        (((x & 0xfff000) >> 12) * 0x1001001001001ULL & 0x84210842108421ULL) % 0x1f;
}

inline uint64_t rank_anderson24(const std::vector<uint64_t>& a) {
    uint64_t rank = 0;
    const size_t n = a.size();
    for(size_t i = 0; i < n; i++) {
        uint64_t x = a[i];
        rank += rank24_anderson(x & 16777215ULL) +
                rank24_anderson((x >> 24ULL) & 16777215ULL) +
                rank24_anderson(x >> 48ULL);
    }
    return rank;
}

// lut8
inline uint64_t rank_lut8(const std::vector<uint64_t>& a) {
    uint64_t rank = 0;
    const size_t n = a.size();
    for(size_t i = 0; i < n; i++) {
        const uint64_t x = a[i];
        rank +=
            uint8_rank_lut[x & 0xFF] +
            uint8_rank_lut[(x >> uint64_t(8)) & 0xFF] +
            uint8_rank_lut[(x >> uint64_t(16)) & 0xFF] +
            uint8_rank_lut[(x >> uint64_t(24)) & 0xFF] +
            uint8_rank_lut[(x >> uint64_t(32)) & 0xFF] +
            uint8_rank_lut[(x >> uint64_t(40)) & 0xFF] +
            uint8_rank_lut[(x >> uint64_t(48)) & 0xFF] +
            uint8_rank_lut[(x >> uint64_t(56)) & 0xFF];
    }
    return rank;
}

// lut16
inline uint64_t rank_lut16(const std::vector<uint64_t>& a) {
    uint64_t rank = 0;
    const size_t n = a.size();
    for(size_t i = 0; i < n; i++) {
        const uint64_t x = a[i];
        rank +=
            uint16_rank_lut[x & 0xFFFF] +
            uint16_rank_lut[(x >> uint64_t(16)) & 0xFFFF] +
            uint16_rank_lut[(x >> uint64_t(32)) & 0xFFFF] +
            uint16_rank_lut[(x >> uint64_t(48)) & 0xFFFF];
    }
    return rank;
}

// popcnt
inline uint64_t rank_popcnt(const std::vector<uint64_t>& a) {
    uint64_t rank = 0;
    const size_t n = a.size();
    for(size_t i = 0; i < n; i++) {
        rank += __builtin_popcountll(a[i]);
    }
    return rank;
}

// popcnt2
inline uint64_t rank_popcnt2(const std::vector<uint64_t>& a) {
    uint64_t rank = 0;
    const size_t n = a.size();
    for(size_t i = 0; i < n; i += 2) {
        rank += __builtin_popcountll(a[i]) + __builtin_popcountll(a[i+1]);
    }
    return rank;
}

// popcnt4
inline uint64_t rank_popcnt4(const std::vector<uint64_t>& a) {
    uint64_t rank = 0;
    const size_t n = a.size();
    size_t i;
    for(i = 0; i < n; i += 4) {
        rank += __builtin_popcountll(a[i]) + __builtin_popcountll(a[i+1]) +
                __builtin_popcountll(a[i+2]) + __builtin_popcountll(a[i+3]);
    }
    return rank;
}

// popcnt8
inline uint64_t rank_popcnt8(const std::vector<uint64_t>& a) {
    uint64_t rank = 0;
    const size_t n = a.size();
    size_t i;
    for(i = 0; i < n; i += 8) {
        rank += __builtin_popcountll(a[i]) + __builtin_popcountll(a[i+1]) +
                __builtin_popcountll(a[i+2]) + __builtin_popcountll(a[i+3]) +
                __builtin_popcountll(a[i+4]) + __builtin_popcountll(a[i+5]) +
                __builtin_popcountll(a[i+6]) + __builtin_popcountll(a[i+7]);
    }
    return rank;
}

// popcnt_movdq from github/WojciechMula/sse-popcount
inline uint64_t rank_popcnt_movdq(const std::vector<uint64_t>& a) {
    uint64_t rank = 0;
    __m128i x, y;

    const size_t n = a.size();
    for(size_t i = 0; i < n; i += 2) {
        x = _mm_load_si128((__m128i*)&a[i]);
        rank += __builtin_popcountll(_mm_cvtsi128_si64(x)); // low 64 bits
        y = (__m128i)_mm_movehl_ps((__m128)x, (__m128)x); // swap h/l
        rank += __builtin_popcountll(_mm_cvtsi128_si64(y)); // high 64 bits
    }
    return rank;
}

// popcnt_movdq from github/WojciechMula/sse-popcount
inline uint64_t rank_popcnt_movdq2(const std::vector<uint64_t>& a) {
    uint64_t rank = 0;
    __m128i x, y, p, q;

    const size_t n = a.size();
    for(size_t i = 0; i < n; i += 4) {
        x = _mm_load_si128((__m128i*)&a[i]);
        y = _mm_load_si128((__m128i*)&a[i+2]);

        // swap h/l
        p = (__m128i)_mm_movehl_ps((__m128)x, (__m128)x);
        q = (__m128i)_mm_movehl_ps((__m128)y, (__m128)y);

        rank += __builtin_popcountll(_mm_cvtsi128_si64(x)) +
                __builtin_popcountll(_mm_cvtsi128_si64(y)) +
                __builtin_popcountll(_mm_cvtsi128_si64(p)) +
                __builtin_popcountll(_mm_cvtsi128_si64(q));
    }
    return rank;
}

// popcnt_movdq from github/WojciechMula/sse-popcount
inline uint64_t rank_popcnt_movdq4(const std::vector<uint64_t>& a) {
    uint64_t rank = 0;
    __m128i x, y, z, w, p, q, r, s;

    const size_t n = a.size();
    for(size_t i = 0; i < n; i += 8) {
        x = _mm_load_si128((__m128i*)&a[i]);
        y = _mm_load_si128((__m128i*)&a[i+2]);
        z = _mm_load_si128((__m128i*)&a[i+4]);
        w = _mm_load_si128((__m128i*)&a[i+6]);

        // swap h/l
        p = (__m128i)_mm_movehl_ps((__m128)x, (__m128)x);
        q = (__m128i)_mm_movehl_ps((__m128)y, (__m128)y);
        r = (__m128i)_mm_movehl_ps((__m128)z, (__m128)z);
        s = (__m128i)_mm_movehl_ps((__m128)w, (__m128)w);

        rank += __builtin_popcountll(_mm_cvtsi128_si64(x)) +
                __builtin_popcountll(_mm_cvtsi128_si64(y)) +
                __builtin_popcountll(_mm_cvtsi128_si64(z)) +
                __builtin_popcountll(_mm_cvtsi128_si64(w)) +
                __builtin_popcountll(_mm_cvtsi128_si64(p)) +
                __builtin_popcountll(_mm_cvtsi128_si64(q)) +
                __builtin_popcountll(_mm_cvtsi128_si64(r)) +
                __builtin_popcountll(_mm_cvtsi128_si64(s));
    }
    return rank;
}

std::vector<uint64_t> generate_queries(size_t num, uint64_t universe, size_t seed = 147ULL) {
    std::vector<uint64_t> queries;
    queries.reserve(num);

    // seed
    std::default_random_engine gen(seed);
    std::uniform_int_distribution<uint64_t> dist(0, universe);

    // generate
    for(size_t i = 0; i < num; i++) {
        queries.push_back(dist(gen));
    }

    return queries;
}

template<typename rank_function_t>
void bench(
    const std::string& name,
    const rapl::reader& r,
    const std::vector<uint64_t>& queries,
    rank_function_t rank) {

    const size_t num = queries.size() - 8;
    
    auto t0 = time();
    auto e0 = r.read();

    const uint64_t sum = rank(queries);

    const auto t = time() - t0;
    const auto e = r.read() - e0;
    std::cout << "RESULT"
              << " name=" << name
              << " t=" << t
              << " e.package=" << e.package
              << " e.core=" << e.core
              << " e.uncore=" << e.uncore
              << " e.dram=" << e.dram
              << " e.psys=" << e.psys
              << " sum=" << sum
              << std::endl;
}

int main(int argc, char** argv) {
    tlx::CmdlineParser cp;

    size_t num = 1'000'000ULL;
    cp.add_bytes('n', "num", num, "The number of queries to perform (default 1M).");

    if (!cp.process(argc, argv)) {
        return -1;
    }

    if(num % 8 != 0) {
        std::cerr << "num must be a multiple of 8!" << std::endl;
        return -2;
    }

    rapl::reader r;

    std::cout << "# Generating queries ..." << std::endl;
    auto queries = generate_queries(num+8, UINT64_MAX, 12345);


    // sum all values to be fair to first benchmark
    uint64_t sum = 0;
    for(size_t i = 0; i < queries.size(); i++) {
        sum += queries[i];
    }
    std::cout << "# Running benchmark (sum=" << sum << ")..." << std::endl;

    bench("shift", r, queries, rank_shift);
    bench("anderson14", r, queries, rank_anderson14);
    bench("anderson24", r, queries, rank_anderson24);
    bench("lut8", r, queries, rank_lut8);
    bench("lut16", r, queries, rank_lut16);
    bench("popcnt", r, queries, rank_popcnt);
    bench("popcnt2", r, queries, rank_popcnt2);
    bench("popcnt4", r, queries, rank_popcnt4);
    bench("popcnt8", r, queries, rank_popcnt8);
    bench("popcnt_movdq", r, queries, rank_popcnt_movdq);
    bench("popcnt_movdq2", r, queries, rank_popcnt_movdq2);
    bench("popcnt_movdq4", r, queries, rank_popcnt_movdq4);
    return 0;
}


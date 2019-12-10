#include <cstdint>
#include <iostream>
#include <random>

#include <stash/rapl/reader.hpp>
#include <stash/rapl/power.hpp>
#include <stash/util/rank8_lut.hpp>
#include <stash/util/rank16_lut.hpp>
#include <stash/util/time.hpp>

#include <tlx/cmdline_parser.hpp>

#include <emmintrin.h>
#include <immintrin.h>

#ifndef __AVX512VL__
#pragma message("AVX-512 VL not supported by CPU")
#endif

using namespace stash;

struct uint128_t { uint64_t a[2]; };
struct uint256_t { uint64_t a[4]; };
struct uint512_t { uint64_t a[8]; };

// lut
template<typename T> uint16_t rank_lut(const T& x);

template<>
inline uint16_t rank_lut(const uint64_t& x) {
    return
        uint8_rank_lut[x & 0xFF] +
        uint8_rank_lut[(x >> uint64_t(8)) & 0xFF] +
        uint8_rank_lut[(x >> uint64_t(16)) & 0xFF] +
        uint8_rank_lut[(x >> uint64_t(24)) & 0xFF] +
        uint8_rank_lut[(x >> uint64_t(32)) & 0xFF] +
        uint8_rank_lut[(x >> uint64_t(40)) & 0xFF] +
        uint8_rank_lut[(x >> uint64_t(48)) & 0xFF] +
        uint8_rank_lut[(x >> uint64_t(56)) & 0xFF];
}

template<>
inline uint16_t rank_lut(const uint128_t& x) {
    return rank_lut(x.a[0]) + rank_lut(x.a[1]);
}

template<>
inline uint16_t rank_lut(const uint256_t& x) {
    return rank_lut(x.a[0])
         + rank_lut(x.a[1])
         + rank_lut(x.a[2])
         + rank_lut(x.a[3]);
}

template<>
inline uint16_t rank_lut(const uint512_t& x) {
    return rank_lut(x.a[0])
         + rank_lut(x.a[1])
         + rank_lut(x.a[2])
         + rank_lut(x.a[3])
         + rank_lut(x.a[4])
         + rank_lut(x.a[5])
         + rank_lut(x.a[6])
         + rank_lut(x.a[7]);
}

// lut2
template<typename T> uint16_t rank_lut2(const T& x);

template<>
inline uint16_t rank_lut2(const uint64_t& x) {
    return
        uint16_rank_lut[x & 0xFFFF] +
        uint16_rank_lut[(x >> uint64_t(16)) & 0xFFFF] +
        uint16_rank_lut[(x >> uint64_t(32)) & 0xFFFF] +
        uint16_rank_lut[(x >> uint64_t(48)) & 0xFFFF];
}

template<>
inline uint16_t rank_lut2(const uint128_t& x) {
    return rank_lut2(x.a[0]) + rank_lut2(x.a[1]);
}

template<>
inline uint16_t rank_lut2(const uint256_t& x) {
    return rank_lut2(x.a[0])
         + rank_lut2(x.a[1])
         + rank_lut2(x.a[2])
         + rank_lut2(x.a[3]);
}

template<>
inline uint16_t rank_lut2(const uint512_t& x) {
    return rank_lut2(x.a[0])
         + rank_lut2(x.a[1])
         + rank_lut2(x.a[2])
         + rank_lut2(x.a[3])
         + rank_lut2(x.a[4])
         + rank_lut2(x.a[5])
         + rank_lut2(x.a[6])
         + rank_lut2(x.a[7]);
}

// popcnt64
template<typename T> uint16_t rank_popcnt64(const T& x);

template<>
inline uint16_t rank_popcnt64(const uint64_t& a) {
    return __builtin_popcountll(a);
}

template<>
inline uint16_t rank_popcnt64(const uint128_t& x) {
    return rank_popcnt64(x.a[0]) + rank_popcnt64(x.a[1]);
}

template<>
inline uint16_t rank_popcnt64(const uint256_t& x) {
    return rank_popcnt64(x.a[0])
         + rank_popcnt64(x.a[1])
         + rank_popcnt64(x.a[2])
         + rank_popcnt64(x.a[3]);
}

template<>
inline uint16_t rank_popcnt64(const uint512_t& x) {
    return rank_popcnt64(x.a[0])
         + rank_popcnt64(x.a[1])
         + rank_popcnt64(x.a[2])
         + rank_popcnt64(x.a[3])
         + rank_popcnt64(x.a[4])
         + rank_popcnt64(x.a[5])
         + rank_popcnt64(x.a[6])
         + rank_popcnt64(x.a[7]);
}

#ifdef __AVX512VL__
// avx512
template<typename T> uint16_t rank_avx512(const T& x);

inline uint16_t rank_avx512(const uint64_t& x) {
    return rank_popcnt64(x);
}

inline uint16_t rank_avx512(const uint128_t& x) {
    __m128i x128 = _mm_set_epi64(x.a[1], x.a[0]);
    return (uint16_t)_mm_popcnt_epi64(x128);
}

inline uint16_t rank_avx512(const uint256_t& a) {
    __m256i x256 = _mm256_set_epi64x(x.a[3], x.a[2], x.a[1], x.a[0]);
    return (uint16_t)_mm256_popcnt_epi64(x256);
}

inline uint16_t rank_avx512(const uint512_t& a) {
    __m512i x512 = _mm512_set_epi64(
        x.a[7], x.a[6], x.a[5], x.a[4],
        x.a[3], x.a[2], x.a[1], x.a[0]);
    return (uint16_t)_mm512_popcnt_epi64(x512);
}
#endif

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

template<typename T> T get(const std::vector<uint64_t>& a, const size_t i);

template<>
inline uint64_t get<uint64_t>(const std::vector<uint64_t>& a, const size_t i) {
    return a[i];
}

template<>
inline uint128_t get<uint128_t>(const std::vector<uint64_t>& a, const size_t i) {
    return uint128_t{a[i], a[i+1]};
}

template<>
inline uint256_t get<uint256_t>(const std::vector<uint64_t>& a, const size_t i) {
    return uint256_t{a[i], a[i+1], a[i+2], a[i+3]};
}

template<>
inline uint512_t get<uint512_t>(const std::vector<uint64_t>& a, const size_t i) {
    return uint512_t{a[i], a[i+1], a[i+2], a[i+3]};
}

template<typename T, typename rank_function_t>
void bench(
    const std::string& name,
    const rapl::reader& r,
    const std::vector<uint64_t>& queries,
    rank_function_t rank) {

    const size_t w = 8 * sizeof(T);

    uint16_t sum = 0;
    const size_t num = queries.size() - 8;
    
    auto t0 = time();
    auto e0 = r.read();
    
    for(size_t i = 0; i < num; i++) {
        sum += rank(get<T>(queries, i));
    }

    const auto t = time() - t0;
    const auto e = r.read() - e0;
    const auto p = rapl::power(e, t);
    std::cout << "RESULT"
              << " name=" << name
              << " w=" << w
              << " t=" << t
              << " e.core=" << e.core
              << " e.uncore=" << e.uncore
              << " e.dram=" << e.dram
              << " e.total=" << e.total()
              << " p.core=" << p.core
              << " p.uncore=" << p.uncore
              << " p.dram=" << p.dram
              << " p.total=" << p.total()
              << " sum=" << sum
              << std::endl;
}

int main(int argc, char** argv) {
    tlx::CmdlineParser cp;

    size_t num = 100'000'000ULL;
    cp.add_bytes('n', "num", num, "The number of queries to perform.");

    if (!cp.process(argc, argv)) {
        return -1;
    }

    rapl::reader r;

    std::cout << "# Generating queries ..." << std::endl;
    auto queries = generate_queries(num+8, UINT64_MAX, 12345);
    std::cout << "# Running benchmark ..." << std::endl;

    // lut
    bench<uint64_t> ("lut", r, queries, rank_lut<uint64_t>);
    bench<uint128_t>("lut", r, queries, rank_lut<uint128_t>);
    bench<uint256_t>("lut", r, queries, rank_lut<uint256_t>);
    bench<uint512_t>("lut", r, queries, rank_lut<uint512_t>);
    // lut2
    bench<uint64_t> ("lut2", r, queries, rank_lut2<uint64_t>);
    bench<uint128_t>("lut2", r, queries, rank_lut2<uint128_t>);
    bench<uint256_t>("lut2", r, queries, rank_lut2<uint256_t>);
    bench<uint512_t>("lut2", r, queries, rank_lut2<uint512_t>);
    // popcnt64
    bench<uint64_t> ("popcnt64", r, queries, rank_popcnt64<uint64_t>);
    bench<uint128_t>("popcnt64", r, queries, rank_popcnt64<uint128_t>);
    bench<uint256_t>("popcnt64", r, queries, rank_popcnt64<uint256_t>);
    bench<uint512_t>("popcnt64", r, queries, rank_popcnt64<uint512_t>);

#ifdef __AVX512VL__
    // avx512
    bench<uint64_t> ("avx512", r, queries, rank_avx512<uint64_t>);
    bench<uint128_t>("avx512", r, queries, rank_avx512<uint128_t>);
    bench<uint256_t>("avx512", r, queries, rank_avx512<uint256_t>);
    bench<uint512_t>("avx512", r, queries, rank_avx512<uint512_t>);
#endif

    return 0;
}


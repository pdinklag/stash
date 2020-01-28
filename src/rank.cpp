#include <cstdint>
#include <iostream>
#include <random>

#include <stash/rapl/reader.hpp>
#include <stash/rapl/power.hpp>
#include <stash/util/rank8_lut.hpp>
#include <stash/util/rank16_lut.hpp>
#include <stash/util/time.hpp>

#include <tlx/cmdline_parser.hpp>

using namespace stash;

// shift
inline uint8_t rank_shift(uint64_t x) {
    uint8_t rank = 0;
    while(x) {
        rank += (x & 1);
        x >>= 1;
    }
    return rank;
}

// lut8
inline uint16_t rank_lut8(uint64_t x) {
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

// lut16
inline uint8_t rank_lut16(uint64_t x) {
    return
        uint16_rank_lut[x & 0xFFFF] +
        uint16_rank_lut[(x >> uint64_t(16)) & 0xFFFF] +
        uint16_rank_lut[(x >> uint64_t(32)) & 0xFFFF] +
        uint16_rank_lut[(x >> uint64_t(48)) & 0xFFFF];
}

// popcnt64
inline uint8_t rank_popcnt64(uint64_t x) {
    return __builtin_popcountll(x);
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

    uint16_t sum = 0;
    const size_t num = queries.size() - 8;
    
    auto t0 = time();
    auto e0 = r.read();
    
    for(size_t i = 0; i < num; i++) {
        sum += rank(queries[i]);
    }

    const auto t = time() - t0;
    const auto e = r.read() - e0;
    const auto p = rapl::power(e, t);
    std::cout << "RESULT"
              << " name=" << name
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

    size_t num = 1'000'000ULL;
    cp.add_bytes('n', "num", num, "The number of queries to perform (default 1M).");

    if (!cp.process(argc, argv)) {
        return -1;
    }

    rapl::reader r;

    std::cout << "# Generating queries ..." << std::endl;
    auto queries = generate_queries(num+8, UINT64_MAX, 12345);
    std::cout << "# Running benchmark ..." << std::endl;

    // lut
    bench("shift", r, queries, rank_shift);
    bench("lut8", r, queries, rank_lut8);
    bench("lut16", r, queries, rank_lut16);
    bench("popcnt64", r, queries, rank_popcnt64);
    return 0;
}


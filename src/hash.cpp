#include <algorithm>
#include <cstdint>
#include <cassert>
#include <iostream>

#include <stash/hash/table.hpp>
#include <stash/hash/linear_probing.hpp>
#include <stash/hash/quadratic_probing.hpp>

#include <stash/io/load_file.hpp>
#include <stash/rapl/reader.hpp>
#include <stash/util/malloc_callback.hpp>
#include <stash/util/random.hpp>
#include <stash/util/time.hpp>

#include <tlx/cmdline_parser.hpp>

using namespace stash;

#ifdef RAPL
rapl::reader raplr;
#endif

namespace malloc_callback {
    size_t current = 0;
    size_t peak = 0;

    void reset() {
        current = 0;
        peak = 0;
    }
    
    void on_alloc(size_t size) {
        current += size;
        peak = std::max(peak, current);
    }
    
    void on_free(size_t size) {
        current -= size;
    }
}


size_t id(uint64_t key) {
    return key;
}

size_t mix(uint64_t key) {
    key = (~key) + (key << 21ULL);
    key = key ^ (key >> 24ULL);
    key = (key + (key << 3ULL)) + (key << 8ULL);
    key = key ^ (key >> 14ULL);
    key = (key + (key << 2ULL)) + (key << 4ULL);
    key = key ^ (key >> 28ULL);
    key = key + (key << 31ULL);
    return key;
}

struct mul_hash {
    uint64_t prime;

    inline size_t operator()(uint64_t key) {
        return key * prime;
    }
};

struct params {
    std::string filename;
    size_t capacity = 0;
    double load_factor = 1.0;
    double growth_factor = 2.0;
    size_t num_queries = 100'000;
};

template<typename hash_func_t, typename probe_func_t>
void test(
    const std::string& name, hash_func_t hfunc, probe_func_t pfunc, const params& p, const std::vector<uint64_t>& keys, const std::vector<uint64_t>& queries) {

    malloc_callback::reset();
    hash::table<uint64_t> h(hfunc, p.capacity, p.load_factor, p.growth_factor, pfunc);
    
    uint64_t t_insert;

    #ifdef RAPL
    uint64_t e_insert;
    uint64_t e0 = raplr.read().total().package;
    #endif
    
    {
        const auto t0 = time();
        for(auto k : keys) {
            h.insert(k);
        }
        t_insert = time() - t0;

        #ifdef RAPL
        e_insert = raplr.read().total().package - e0;
        #endif
    }

    const auto m = malloc_callback::current;
    const auto mpeak = malloc_callback::peak;
    const auto mratio = (double)m / (double)(keys.size() * sizeof(uint64_t));

    size_t chksum = 0;
    uint64_t t_member;
    
    #ifdef RAPL
    uint64_t e_member;
    e0 = raplr.read().total().package;
    #endif
    
    {
        const auto t0 = time();
        for(size_t i = 0; i < p.num_queries; i++) {
            chksum += h.contains(keys[queries[i]]);
        }
        t_member = time() - t0;
        
        #ifdef RAPL
        e_member = raplr.read().total().package - e0;
        #endif
    }

    std::cout
        << "RESULT"
        << " hfunc=" << name
        << " m=" << m
        << " t_insert=" << t_insert
        << " t_member=" << t_member;
        
    #ifdef RAPL
    std::cout
        << " e_insert=" << e_insert
        << " e_member=" << e_member;
    #endif

    std::cout
        << " queries=" << p.num_queries
        << " chksum=" << chksum
        << " mratio=" << mratio
        << " mpeak=" << mpeak
        << " size=" << h.size()
        << " cap=" << h.capacity()
        << " load=" << h.load()
        << " max_probe=" << h.max_probe()
        << " avg_probe=" << h.avg_probe()
        << " resizes=" << h.times_resized()
        << std::endl;
}

int main(int argc, char** argv) {
    tlx::CmdlineParser cp;

    params p;
    cp.add_param_string("file", p.filename, "the integer keys to insert");
    cp.add_bytes('c', "capacity", p.capacity, "the initial capacity (default: input size)");
    cp.add_double('l', "load-factor", p.load_factor, "the maximum load factor (default: 1)");
    cp.add_double('g', "growth-factor", p.growth_factor, "the growth factor (default: 2)");
    cp.add_bytes('q', "queries", p.num_queries, "the number of membership queries to perform");
    
    if (!cp.process(argc, argv)) {
        return -1;
    }

    auto keys = io::load_file_lines_as_vector<uint64_t>(p.filename);
    auto queries = random_vector<size_t>(p.num_queries, keys.size() - 1);

    if(p.capacity == 0) {
        p.capacity = keys.size();
    }

    test("lp1.knuth      ", mul_hash{2654435761ULL},                 hash::linear_probing<>{}, p, keys, queries);
    test("lp1.mul_prime1 ", mul_hash{15'425'459'083'914'370'367ULL}, hash::linear_probing<>{}, p, keys, queries);
    test("lp1.mul_prime2 ", mul_hash{16'568'458'216'213'224'001ULL}, hash::linear_probing<>{}, p, keys, queries);
    test("lp1.mul_prime3 ", mul_hash{17'406'548'584'874'384'839ULL}, hash::linear_probing<>{}, p, keys, queries);
    test("lp1.mix        ", mix,                                     hash::linear_probing<>{}, p, keys, queries);
    
    test("lp7.knuth      ", mul_hash{2654435761ULL},                 hash::linear_probing<7>{}, p, keys, queries);
    test("lp7.mul_prime1 ", mul_hash{15'425'459'083'914'370'367ULL}, hash::linear_probing<7>{}, p, keys, queries);
    test("lp7.mul_prime2 ", mul_hash{16'568'458'216'213'224'001ULL}, hash::linear_probing<7>{}, p, keys, queries);
    test("lp7.mul_prime3 ", mul_hash{17'406'548'584'874'384'839ULL}, hash::linear_probing<7>{}, p, keys, queries);
    test("lp7.mix        ", mix,                                     hash::linear_probing<7>{}, p, keys, queries);
    
    test("qp11.knuth     ", mul_hash{2654435761ULL},                 hash::quadratic_probing<>{}, p, keys, queries);
    test("qp11.mul_prime1", mul_hash{15'425'459'083'914'370'367ULL}, hash::quadratic_probing<>{}, p, keys, queries);
    test("qp11.mul_prime2", mul_hash{16'568'458'216'213'224'001ULL}, hash::quadratic_probing<>{}, p, keys, queries);
    test("qp11.mul_prime3", mul_hash{17'406'548'584'874'384'839ULL}, hash::quadratic_probing<>{}, p, keys, queries);
    test("qp11.mix       ", mix,                                     hash::quadratic_probing<>{}, p, keys, queries);
    
    return 0;
}

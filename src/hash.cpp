#include <cstdint>
#include <cassert>
#include <iostream>

#include <stash/hash/table.hpp>
#include <stash/io/load_file.hpp>
#include <stash/util/time.hpp>

#include <tlx/cmdline_parser.hpp>

using namespace stash;

constexpr uint64_t prime = 14'667'900'939'675'899'869ULL;

inline size_t simple_hash(uint64_t key) {
    return key * prime;
}

int main(int argc, char** argv) {
    tlx::CmdlineParser cp;

    std::string filename;
    cp.add_param_string("file", filename, "the integer keys to insert");

    size_t capacity = 0;
    cp.add_bytes('c', "capacity", capacity, "the initial capacity");

    double load_factor = 1.0;
    cp.add_double('l', "load-factor", load_factor, "the maximum load factor");

    double growth_factor = 2.0;
    cp.add_double('g', "growth-factor", growth_factor, "the growth factor");
    
    if (!cp.process(argc, argv)) {
        return -1;
    }
        
    auto keys = io::load_file_lines_as_vector<uint64_t>(filename);

    if(capacity == 0) {
        capacity = keys.size();
    }

    hash::table<uint64_t> h(simple_hash, capacity, load_factor, growth_factor);
    auto t0 = time();
    for(auto k : keys) {
        h.insert(k);
    }
    auto dt = time() - t0;

    for(auto k : keys) {
        assert(h.contains(k));
    }

    std::cout << "t=" << dt << ", size=" << h.size() << ", capacity=" << h.capacity() << ", load=" << h.load() << ", max_probe=" << h.max_probe() << std::endl;
    return 0;
}

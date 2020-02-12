#include <algorithm>
#include <bitset>
#include <cassert>
#include <iostream>
#include <random>
#include <vector>

#include <stash/util/math.hpp>
#include <stash/util/time.hpp>
#include <stash/util/random.hpp>

#include <tlx/cmdline_parser.hpp>

using namespace stash;

int main(int argc, char** argv) {
    tlx::CmdlineParser cp;

    uint32_t num = 10ULL;
    cp.add_bytes('n', "num", num, "The number of numbers to generate (default: 10).");

    size_t u = 0xFFFFFFFFULL;
    cp.add_bytes('u', "universe", u, "The universe to draw numbers from (default: 32 bits).");

    size_t seed = time_nanos();
    cp.add_size_t('s', "seed", seed, "The seed for random generation (default: timestamp).");

    std::string mode = "qred";
    cp.add_string('m', "mode", mode, "The mode to use ('qred' for quadratic residue, 'add' for addition-based)");
    
#ifndef NDEBUG
    bool check = false;
    cp.add_flag('c', "check", check, "Check that a permutation is generated (debug).");
#endif

    if(!cp.process(argc, argv)) {
        return -1;
    }

    if(u < num) {
        std::cerr << "the universe must be at least as large as the number of generated numbers" << std::endl;
        return -2;
    }

    // generate numbers
    if(mode == "qred") {
        auto perm = random::permutation(u, seed);

    #ifndef NDEBUG
        if(check) {
            std::vector<bool> b(u);
            for(uint64_t i = 0; i < u; i++) {
                const uint64_t j = perm(i);
                assert(!b[j]);
                b[j] = 1;
            }
        }
    #endif

        for(uint64_t i = 0; i < num; i++) {
            std::cout << perm(i) << std::endl;
        }
    } else if(mode == "add") {
        const auto add_max = u / num;
        std::default_random_engine gen(seed);
        std::uniform_int_distribution<uint64_t> init(0, add_max+1);
        std::uniform_int_distribution<uint64_t> dist(1, add_max+1);

        auto next = init(gen);
        for(uint64_t i = 0; i < num; i++) {
            std::cout << next << std::endl;
            next += dist(gen);
        }
    }
    
    return 0;
}

#include <algorithm>
#include <bitset>
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

    if(!cp.process(argc, argv)) {
        return -1;
    }

    if(u < num) {
        std::cerr << "the universe must be at least as large as the number of generated numbers" << std::endl;
        return -2;
    }

    // generate numbers
    auto perm = random::permutation(u, seed);
    for(uint64_t i = 0; i < num; i++) {
        std::cout << perm(i) << std::endl;
    }
    return 0;
}

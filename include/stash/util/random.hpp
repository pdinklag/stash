#pragma once

#include <random>
#include <vector>

namespace stash {
    // generate a random vector from 0 to universe (inclusive)
    template<typename T>
    std::vector<T> random_vector(size_t num, T universe, size_t seed = 147ULL) {
        std::vector<T> v;
        v.reserve(num);

        // seed
        std::default_random_engine gen(seed);
        std::uniform_int_distribution<T> dist(0, universe);

        // generate
        for(size_t i = 0; i < num; i++) {
            v.push_back(dist(gen));
        }

        return v;
    }
}

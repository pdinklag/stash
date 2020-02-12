#pragma once

#include <random>
#include <vector>

#include <stash/util/math.hpp>
#include <stash/util/time.hpp>
#include <stash/util/uint128.hpp>

namespace stash {
    namespace random {
        // generates a random permutation from 0 to u-1
        // based on an article by Jeff Preshing
        // https://preshing.com/20121224/how-to-generate-a-sequence-of-unique-random-integers/
        //
        // modified for any universe size (not just 32 bits)
        class permutation {
        private:
            // decent distribution of 64 bits
            static constexpr uint64_t SHUFFLE1 = 0x9696594B6A5936B2ULL;
            static constexpr uint64_t SHUFFLE2 = 0xD2165B4B66592AD6ULL;

            static inline uint64_t prev_prime_3mod4(const uint64_t x) {
                uint64_t p = prime_predecessor(x);
                while((p & 3) != 3) {
                    p = prime_predecessor(p-1);
                }
                return p;
            }

            const uint64_t m_universe;
            const uint64_t m_seed;
            const uint64_t m_prime;

            inline uint64_t permute(const uint64_t x) const {
                if(x >= m_prime) {
                    // map numbers in gap to themselves - shuffling will take care of this
                    return x;
                } else {
                    // use quadratic residue
                    uint64_t r = (uint64_t)(((uint128_t)x * (uint128_t)x) % (uint128_t)m_prime);
                    return (x <= (m_prime >> 1ULL)) ? r : m_prime - r;
                }
            }

        public:
            permutation(uint64_t universe, uint64_t seed)
                : m_universe(universe),
                  m_prime(prev_prime_3mod4(universe)),
                  m_seed((seed ^ SHUFFLE1) ^ SHUFFLE2) {
            }

            permutation(uint64_t universe) : permutation(universe, time_nanos()) {
            }

            inline uint64_t operator()(uint64_t i) const {
                return permute((m_seed + permute(i)) % m_universe);
            }

            inline std::vector<uint64_t> vector(size_t num) const {
                std::vector<uint64_t> vec(num);
                for(size_t i = 0; i < num; i++) {
                    vec[i] = (*this)(i);
                }
                return vec;
            }
        };
    };
    
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

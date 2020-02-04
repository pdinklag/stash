#pragma once

#include <array>
#include <stash/rapl/config.hpp>
#include <stash/rapl/energy.hpp>

namespace stash {
namespace rapl {

    struct energy_buffer {

        
        std::array<energy, max_rapl_packages> packages;

        inline energy_buffer() {
        }

        inline energy& operator[](size_t i) {
            return packages[i];
        }

        inline const energy& operator[](size_t i) const {
            return packages[i];
        }

        inline energy_buffer operator-(const energy_buffer& other) {
            energy_buffer b;
            for(size_t i = 0; i < max_rapl_packages; i++) {
                b[i] = packages[i] - other.packages[i];
            }
            return b;
        }

        inline energy total() {
            energy e;
            for(size_t i = 0; i < max_rapl_packages; i++) {
                e += packages[i];
            }
            return e;
        }
    };

}}

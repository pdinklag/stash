#pragma once

#include <cassert>
#include <util/likely.hpp>

template<typename array_t>
void assert_sorted_ascending(const array_t& a) {
    #ifndef NDEBUG
    const size_t num = a.size();
    if(num > 1) {                
        auto prev = a[0];
        for(size_t i = 1; i < num; i++) {
            auto next = a[i];
            assert(prev < next);
            prev = next;
        }
    }
    #endif
}

#pragma once

#include <cstdint>

namespace stash {
namespace hash {

template<size_t m_inc = 1>
struct linear_probing {
    inline size_t operator()(const size_t i) {
        return i + m_inc;
    }
};

}}

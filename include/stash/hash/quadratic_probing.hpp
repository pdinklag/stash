#pragma once

#include <cstdint>

namespace stash {
namespace hash {

template<size_t m_a = 1, size_t m_b = 1, size_t m_inc = 1>
struct quadratic_probing {
    inline size_t operator()(const size_t i) {
        return m_a * i * i + m_b * i + m_inc;
    }
};

}}

#pragma once

#include <algorithm>
#include <utility>

#include <stash/pred/util.hpp>
#include <stash/pred/result.hpp>

#include <stash/pred/binsearch_cache.hpp>

#include <stash/vec/int_vector.hpp>
#include <stash/util/math.hpp>

namespace stash {
namespace pred {

template<
    typename array_t,
    typename item_t,
    size_t m_lo_bits,
    size_t m_cache_num = 512ULL / sizeof(item_t)>
class index {
private:
    static constexpr size_t m_hi_bits = 8 * sizeof(item_t) - m_lo_bits;

    static constexpr uint64_t hi(uint64_t x) {
        return x >> m_lo_bits;
    }

    const array_t* m_array;
    size_t m_num;
    item_t m_min;
    item_t m_max;

    uint64_t m_key_min;
    uint64_t m_key_max;

    int_vector m_hi_idx;

    using lo_pred_t = binsearch_cache<array_t, item_t, m_cache_num>;
    lo_pred_t m_lo_pred;

public:
    inline index() : m_num(0), m_min(0), m_max(0), m_array(nullptr) {
    }

    inline index(const index& other) {
        *this = other;
    }

    inline index(index&& other) {
        *this = std::move(other);
    }

    inline index(const array_t& array)
        : m_num(array.size()),
          m_min(array[0]),
          m_max(array[m_num-1]),
          m_array(&array) {

        assert_sorted_ascending(array);

        // build an index for high bits
        m_key_min = uint64_t(m_min) >> m_lo_bits;
        m_key_max = uint64_t(m_max) >> m_lo_bits;

        m_hi_idx = int_vector(m_key_max - m_key_min + 2, log2_ceil(m_num-1));
        m_hi_idx[0] = 0;
        uint64_t prev_key = m_key_min;
        for(size_t i = 1; i < m_num; i++) {
            const uint64_t cur_key = hi(array[i]);
            if(cur_key > prev_key) {
                for(uint64_t key = prev_key + 1; key <= cur_key; key++) {
                    m_hi_idx[key - m_key_min] = i - 1;
                }
            }
            prev_key = cur_key;
        }

        assert(prev_key == m_key_max);
        m_hi_idx[m_key_max - m_key_min] = m_num - 1;

        // build the predecessor data structure for low bits
        m_lo_pred = lo_pred_t(array);
    }

    inline index& operator=(const index& other) {
        m_array = other.m_array;
        m_num = other.m_num;
        m_min = other.m_min;
        m_max = other.m_max;
        m_key_min = other.m_key_min;
        m_key_max = other.m_key_max;
        m_hi_idx = other.m_hi_idx;
        m_lo_pred = other.m_lo_pred;
        return *this;
    }

    inline index& operator=(index&& other) {
        m_array = other.m_array;
        m_num = other.m_num;
        m_min = other.m_min;
        m_max = other.m_max;
        m_key_min = other.m_key_min;
        m_key_max = other.m_key_max;
        m_hi_idx = std::move(other.m_hi_idx);
        m_lo_pred = std::move(other.m_lo_pred);
        return *this;
    }

    inline result predecessor(const item_t x) const {
        if(unlikely(x < m_min))  return result { false, 0 };
        if(unlikely(x >= m_max)) return result { true, m_num-1 };

        const uint64_t key = hi(x) - m_key_min;
        const size_t q = m_hi_idx[key+1];

        if(unlikely(x == (*m_array)[q])) {
            return result { true, q };
        } else {
            const size_t p = m_hi_idx[key];
            return m_lo_pred.predecessor_seeded(x, p, q);
        }
    }

    inline result successor(const item_t x) const {
        if(unlikely(x <= m_min)) return result { true, 0 };
        if(unlikely(x > m_max))  return result { false, 0 };

        const uint64_t key = hi(x) - m_key_min;
        const size_t _q = m_hi_idx[key+1] + 1;
        const size_t q = _q - (_q >= m_num); // std::min(_q, m_num - 1);

        if(unlikely(x == (*m_array)[q])) {
            return result { true, q };
        } else {
            const size_t p = m_hi_idx[key] + 1;
            return m_lo_pred.successor_seeded(x, p, q);
        }
    }
};

}}

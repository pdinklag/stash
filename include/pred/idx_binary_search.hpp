#pragma once

#include <algorithm>

#include <pred/util.hpp>
#include <pred/result.hpp>

#include <pred/binary_search_cache.hpp>

#include <vec/int_vector.hpp>
#include <util/math.hpp>

namespace pred {

template<
    typename array_t,
    typename item_t,
    size_t m_lo_bits,
    size_t m_cache_num = 512ULL / sizeof(item_t)>
class idx_binary_search {
private:
    static constexpr size_t m_hi_bits = 8 * sizeof(item_t) - m_lo_bits;

    const array_t* m_array;
    size_t m_num;
    item_t m_min;
    item_t m_max;

    uint64_t m_key_min;
    uint64_t m_key_max;

    int_vector m_hi_idx;

    using lo_pred_t = binary_search_cache<array_t, item_t, m_cache_num>;
    lo_pred_t m_lo_pred;

public:
    inline idx_binary_search(const array_t& array)
        : m_num(array.size()),
          m_min(array[0]),
          m_max(array[m_num-1]),
          m_array(&array) {

        assert_sorted_ascending(array);

        // build an index for high bits
        m_key_min = uint64_t(m_min) >> m_lo_bits;
        m_key_max = uint64_t(m_max) >> m_lo_bits;

        m_hi_idx = int_vector(m_key_max - m_key_min, log2_ceil(m_num-1));

        uint64_t prev_key = m_key_min;
        for(size_t i = 1; i < m_num; i++) {
            uint64_t key = array[i] >> m_lo_bits;
            if(key != prev_key) {
                for(uint64_t k = prev_key; k < key; k++) {
                    //m_hi_idx.emplace_back(i);
                    m_hi_idx[k - m_key_min] = i;
                }

                prev_key = key;
            }
        }

        assert(prev_key == m_key_max);

        // build the predecessor data structure for low bits
        m_lo_pred = lo_pred_t(array);
    }

    inline result<item_t> predecessor(const item_t x) const {
        if(unlikely(x < m_min))  return result<item_t> { false, false, x };
        if(unlikely(x >= m_max)) return result<item_t> { true, false, m_max };
        
        const uint64_t key = x >> m_lo_bits;

        const size_t q = (key == m_key_max ? m_num-1 : m_hi_idx[key - m_key_min]);
        assert(x < (*m_array)[q]);
        if(unlikely(x == (*m_array)[q])) {
            return result<item_t> { true, true, x };
        } else {
            const size_t p = (key == m_key_min ? 0 : m_hi_idx[key-1-m_key_min]);
            return m_lo_pred.predecessor_seeded(x, p, q);
        }
    }
};

}

#pragma once

#include <algorithm>

#include <pred/util.hpp>
#include <pred/result.hpp>

#include <pred/binary_search_cache.hpp>

#include <vec/bit_vector.hpp>
#include <vec/bit_rank.hpp>
#include <vec/int_vector.hpp>
#include <util/math.hpp>

namespace pred {

template<
    typename array_t,
    typename item_t,
    size_t m_lo_bits,
    size_t m_cache_num = 512ULL / sizeof(item_t)>
class idx_binary_search_sparse {
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
    bit_vector m_hi_bv;
    bit_rank   m_hi_rank;

    using lo_pred_t = binary_search_cache<array_t, item_t, m_cache_num>;
    lo_pred_t m_lo_pred;

public:
    inline idx_binary_search_sparse(const array_t& array)
        : m_num(array.size()),
          m_min(array[0]),
          m_max(array[m_num-1]),
          m_array(&array) {

        assert_sorted_ascending(array);

        // build an index for high bits
        m_key_min = uint64_t(m_min) >> m_lo_bits;
        m_key_max = uint64_t(m_max) >> m_lo_bits;

        size_t cap = m_key_max - m_key_min + 1;
        size_t used = 0;

        m_hi_idx = int_vector(cap, log2_ceil(m_num-1));
        m_hi_idx[used++] = 0;

        m_hi_bv = bit_vector(cap);
        
        uint64_t prev_key = m_key_min;
        for(size_t i = 1; i < m_num; i++) {
            const uint64_t cur_key = hi(array[i]);
            if(cur_key > prev_key) {
                m_hi_idx[used++] = i-1;
                m_hi_bv[prev_key + 1 - m_key_min] = 1;
                //~ for(uint64_t key = prev_key + 1; key <= cur_key; key++) {
                    //~ m_hi_idx[key - m_key_min] = i - 1;
                //~ }
            }
            prev_key = cur_key;
        }

        assert(prev_key == m_key_max);

        // compress
        m_hi_idx[used++] = m_num - 1;
        m_hi_bv[m_key_max - m_key_min] = 1;
        
        if(used < cap) {
            m_hi_idx.rebuild(used);
        }

        // rank support
        m_hi_rank = bit_rank(m_hi_bv);

        // build the predecessor data structure for low bits
        m_lo_pred = lo_pred_t(array);
    }

    inline result<item_t> predecessor(const item_t x) const {
        if(unlikely(x < m_min))  return result<item_t> { false, false, x };
        if(unlikely(x >= m_max)) return result<item_t> { true, false, m_max };
        
        const uint64_t key = hi(x) - m_key_min;
        const size_t q = m_hi_idx[m_hi_rank(key+1)-1];

        if(unlikely(x == (*m_array)[q])) {
            return result<item_t> { true, true, x };
        } else {
            const size_t p = key > 0 ? m_hi_idx[m_hi_rank(key)-1] : 0;
            return m_lo_pred.predecessor_seeded(x, p, q);
        }
    }
};

}
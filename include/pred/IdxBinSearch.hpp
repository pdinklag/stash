#pragma once

#include <algorithm>

#include <pred/util.hpp>
#include <pred/Result.hpp>

#include <pred/CacheBinSearch.hpp>

#include <vec/util.hpp>

namespace pred {

template<
    typename array_t,
    typename item_t,
    size_t m_lo_bits,
    size_t m_cache_num = 512ULL / sizeof(item_t)>
class IdxBinSearch {
private:
    static constexpr size_t m_hi_bits = 8 * sizeof(item_t) - m_lo_bits;

    const array_t* m_array;
    size_t m_num;
    item_t m_min;
    item_t m_max;

    uint64_t m_key_min;
    uint64_t m_key_max;

    std::vector<size_t> m_hi_idx;

    using lo_pred_t = CacheBinSearch<array_t, item_t, m_cache_num>;
    lo_pred_t m_lo_pred;

public:
    inline IdxBinSearch(const array_t& array)
        : m_num(array.size()),
          m_min(array[0]),
          m_max(array[m_num-1]),
          m_array(&array) {

        assert_sorted_ascending(array);

        // build an index for high bits
        m_key_min = uint64_t(m_min) >> m_lo_bits;
        m_key_max = uint64_t(m_max) >> m_lo_bits;

        m_hi_idx.reserve(m_key_max - m_key_min + 1);

        uint64_t prev_key = m_key_min;
        size_t cur_interval_start = 0;

        for(size_t i = 1; i < m_num; i++) {
            uint64_t key = array[i] >> m_lo_bits;
            if(key != prev_key) {
                for(uint64_t k = prev_key; k < key; k++) {
                    m_hi_idx.emplace_back(i);
                }

                prev_key = key;
                cur_interval_start = i-1; // include the last value with the
                                          // previous key (important)!
            }
        }

        assert(m_hi_idx.size() == m_hi_idx.capacity());
        assert(prev_key == m_key_max);

        // build the predecessor data structure for low bits
        m_lo_pred = lo_pred_t(array);
    }

    inline Result<item_t> predecessor(const item_t x) const {
        if(__builtin_expect(x < m_min, false))  return Result<item_t> { false, false, x };
        if(__builtin_expect(x >= m_max, false)) return Result<item_t> { true, x == m_max, m_max };

        const uint64_t key = x >> m_lo_bits;

        const size_t q = (key == m_key_max ? m_num-1 : m_hi_idx[key - m_key_min]);
        if(__builtin_expect(x == (*m_array)[q], false)) {
            return Result<item_t> { true, true, x };
        } else {
            const size_t p = (key == m_key_min ? 0 : m_hi_idx[key-1-m_key_min]);
            return m_lo_pred.predecessor_seeded(x, p, q);
        }
    }
};

}

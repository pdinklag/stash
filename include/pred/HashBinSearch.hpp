#pragma once

#include <algorithm>
#include <unordered_map>

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
class HashBinSearch {
private:
    static constexpr size_t m_hi_bits = 8 * sizeof(item_t) - m_lo_bits;

    const array_t* m_array;
    size_t m_num;
    item_t m_min;
    item_t m_max;

    struct Interval {
        size_t first, last;

        inline Interval(size_t _first, size_t _last) : first(_first), last(_last) {}
    };

    std::vector<Interval> m_idx_intervals;
    std::unordered_map<uint64_t, size_t> m_hi_idx;

    using lo_pred_t = CacheBinSearch<array_t, item_t>;
    lo_pred_t m_lo_pred;

public:
    inline HashBinSearch(const array_t& array)
        : m_num(array.size()),
          m_min(array[0]),
          m_max(array[m_num-1]),
          m_array(&array) {

        assert_sorted_ascending(array);

        // build an index for high bits
        const uint64_t min_key = uint64_t(m_min) >> m_lo_bits;
        const uint64_t max_key = uint64_t(m_max) >> m_lo_bits;
        m_hi_idx.reserve(max_key - min_key + 1);

        uint64_t prev_key = min_key;
        size_t cur_interval_start = 0;
        size_t cur_interval_num = 0;

        for(size_t i = 1; i < m_num; i++) {
            uint64_t key = array[i] >> m_lo_bits;
            if(key != prev_key) {
                m_idx_intervals.emplace_back(cur_interval_start, i);

                for(uint64_t k = prev_key; k < key; k++) {
                    m_hi_idx.emplace(k, cur_interval_num);
                }

                prev_key = key;
                ++cur_interval_num;
                cur_interval_start = i-1; // include the last value with the
                                          // previous key (important)!
            }
        }

        assert(prev_key == max_key);

        m_idx_intervals.emplace_back(cur_interval_start, m_num-1);

        m_hi_idx.emplace(prev_key, cur_interval_num);
        m_hi_idx.rehash(0);

        // DEBUG
        #ifndef NDEBUG
        std::cout << "# HashBinSearch: keys=" << m_hi_idx.size()
                  << ", load_factor=" << m_hi_idx.load_factor()
                  << std::endl;
        #endif

        // build the predecessor data structure for low bits
        m_lo_pred = lo_pred_t(array);
    }

    inline ~HashBinSearch() {
    }

    inline Result<item_t> predecessor(const item_t x) const {
        if(__builtin_expect(x < m_min, false))  return Result<item_t> { false, false, x };
        if(__builtin_expect(x >= m_max, false)) return Result<item_t> { true, x == m_max, m_max };

        uint64_t key = x >> m_lo_bits;
        auto it = m_hi_idx.find(key);
        assert(it != m_hi_idx.end());

        const auto& interval = m_idx_intervals[it->second];
        if(__builtin_expect(x == (*m_array)[interval.last], false)) {
            return Result<item_t> { true, true, x };
        } else {
            return m_lo_pred.predecessor_seeded(x, interval.first, interval.last);
        }
    }
};

}

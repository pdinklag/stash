#pragma once

#include <algorithm>

#include <pred/util.hpp>
#include <pred/result.hpp>

#include <pred/binary_search_cache.hpp>

#include <util/math.hpp>

namespace pred {

template<
    typename array_t,
    typename item_t,
    size_t m_k,
    size_t m_cache_num = 512ULL / sizeof(item_t)>
class two_level_binary_search {
private:
    const array_t* m_array;
    size_t m_num;
    item_t m_min;
    item_t m_max;

    struct sample {
        item_t item;
        size_t pos;

        inline sample() : item(0), pos(0) {
        }
        
        inline sample(item_t _item, size_t _pos) : item(_item), pos(_pos) {
        }

        inline bool operator < (const sample& x) const { return item <  x.item; }
        inline bool operator <=(const sample& x) const { return item <= x.item; }
        inline bool operator ==(const sample& x) const { return item == x.item; }
        inline bool operator !=(const sample& x) const { return item != x.item; }
        inline bool operator >=(const sample& x) const { return item >= x.item; }
        inline bool operator > (const sample& x) const { return item >  x.item; }
    };
    using idx_pred_t = binary_search_cache<std::vector<sample>, sample>;

    std::vector<sample> m_sample;
    idx_pred_t          m_idx;

public:
    inline two_level_binary_search(const array_t& array)
        : m_num(array.size()),
          m_min(array[0]),
          m_max(array[m_num-1]),
          m_array(&array) {

        assert_sorted_ascending(array);

        // sample
        const size_t num_samples = idiv_ceil(m_num, m_k);
        
        m_sample.reserve(num_samples);
        for(size_t i = 0; i < m_num; i += m_k) {
            m_sample.emplace_back(array[i], i);
        }

        m_idx = idx_pred_t(m_sample);
    }

    inline result<item_t> predecessor(const item_t x) const {
        if(unlikely(x < m_min))  return result<item_t> { false, false, x };
        if(unlikely(x >= m_max)) return result<item_t> { true, false, m_max };

        auto idx = m_idx.predecessor(sample{x, 0}).value;
        if(idx.item == x) {
            return result<item_t> { true, true, x };
        } else {
            size_t p = idx.pos;
            size_t q = std::min(idx.pos + m_k, m_num - 1);

            // binary search
            while(q - p > m_cache_num) {
                assert(x >= (*m_array)[p]);
                assert(x < (*m_array)[q]);
                
                const size_t m = (p + q) >> 1ULL;
                
                const bool le = ((*m_array)[m] <= x);
                const size_t le_mask = -size_t(le);
                const size_t gt_mask = ~le_mask;

                if(le) assert(le_mask == SIZE_MAX && gt_mask == 0ULL);
                else   assert(gt_mask == SIZE_MAX && le_mask == 0ULL);
                
                p = (le_mask & m) | (gt_mask & p);
                q = (gt_mask & m) | (le_mask & q);
            }

            // linear search
            while((*m_array)[p] <= x) ++p;
            assert((*m_array)[p-1] <= x);
            
            const item_t r = (*m_array)[p - 1];
            return result<item_t> { true, r == x, r };
        }
    }
};

}

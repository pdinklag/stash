#pragma once

#include <pred/util.hpp>
#include <pred/Result.hpp>

namespace pred {

template<typename array_t, typename item_t, size_t m_cache_num = 512ULL / sizeof(item_t)>
class CacheBinSearch {
private:
    const array_t* m_array;
    size_t m_num;
    item_t m_min;
    item_t m_max;

public:
    inline CacheBinSearch() : m_array(nullptr), m_num(0), m_min(), m_max() {
    }

    inline CacheBinSearch(CacheBinSearch&& other) {
        *this = other;
    }
    
    inline CacheBinSearch(const CacheBinSearch& other) {
        *this = other;
    }

    inline CacheBinSearch(const array_t& array)
        : m_num(array.size()),
          m_min(array[0]),
          m_max(array[m_num-1]),
          m_array(&array) {

        assert_sorted_ascending(array);
    }

    inline CacheBinSearch& operator=(CacheBinSearch&& other) {
        m_array = other.m_array;
        m_num = other.m_num;
        m_min = other.m_min;
        m_max = other.m_max;
        return *this;
    }

    inline CacheBinSearch& operator=(const CacheBinSearch& other) {
        m_array = other.m_array;
        m_num = other.m_num;
        m_min = other.m_min;
        m_max = other.m_max;
        return *this;
    }

    inline Result<item_t> predecessor_seeded(
        const item_t x, size_t p, size_t q) const {

        assert(x >= m_min && x < m_max);
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
        return Result<item_t> { true, r == x, r };
    }

    inline Result<item_t> predecessor(const item_t x) const {
        if(__builtin_expect(x < m_min, false))  return Result<item_t> { false, false, x };
        if(__builtin_expect(x >= m_max, false)) return Result<item_t> { true, false, m_max };
        return predecessor_seeded(x, 0, m_num - 1);
    }
};

}

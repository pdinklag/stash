#pragma once

#include <pred/result.hpp>
#include <pred/util.hpp>

namespace pred {

template<typename array_t, typename item_t>
class binsearch {
private:
    const array_t* m_array;
    size_t m_num;
    item_t m_min;
    item_t m_max;

public:
    inline binsearch() : m_array(nullptr), m_num(0), m_min(), m_max() {
    }

    inline binsearch(binsearch&& other) {
        *this = other;
    }
    
    inline binsearch(const binsearch& other) {
        *this = other;
    }

    inline binsearch(const array_t& array)
        : m_num(array.size()),
          m_min(array[0]),
          m_max(array[m_num-1]),
          m_array(&array) {

        assert_sorted_ascending(array);
    }

    inline binsearch& operator=(binsearch&& other) {
        m_array = other.m_array;
        m_num = other.m_num;
        m_min = other.m_min;
        m_max = other.m_max;
        return *this;
    }

    inline binsearch& operator=(const binsearch& other) {
        m_array = other.m_array;
        m_num = other.m_num;
        m_min = other.m_min;
        m_max = other.m_max;
        return *this;
    }

    inline result<item_t> predecessor(const item_t x) const {
        if(unlikely(x < m_min))  return result<item_t> { false, false, x };
        if(unlikely(x >= m_max)) return result<item_t> { true, false, m_max };
        
        size_t p = 0;
        size_t q = m_num - 1;

        while(p < q - 1) {
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

        const item_t r = (*m_array)[p];
        return result<item_t> { true, r == x, r };
    }
};

}

#pragma once

#include <cassert>

namespace pred {

template<typename array_t, typename item_t>
class BinSearch {
private:
    array_t m_array;
    size_t m_num;
    item_t m_min;
    item_t m_max;

    inline void verify_sorted() {
    #ifndef NDEBUG
        if(m_num > 1) {                
            auto prev = m_array[0];
            for(size_t i = 1; i < m_array.size(); i++) {
                auto next = m_array[i];
                assert(prev < next);
                prev = next;
            }
        }
    #endif
    }

public:
    struct Result {
        bool   exists;
        bool   contained;
        item_t value;

        inline operator bool() const {
            return exists;
        }

        inline operator item_t() const {
            return value;
        }
    };

    inline BinSearch(const array_t& array)
        : m_num(array.size()),
          m_min(array[0]),
          m_max(array[m_num-1]),
          m_array(array) {

        verify_sorted();
    }

    inline BinSearch(array_t&& array) :
        m_num(array.size()),
        m_min(array[0]),
        m_max(array[m_num-1]),
        m_array(array) {

        verify_sorted();
    }

    inline Result predecessor(const item_t x) const {
        if(__builtin_expect(x < m_min, false)) return Result { false, false, x };
        if(__builtin_expect(x >= m_max, false)) return Result { true, false, m_max };
        
        size_t p = 0;
        size_t q = m_num - 1;

        while(p < q - 1) {
            assert(x >= m_array[p]);
            assert(x < m_array[q]);
            
            const size_t m = (p + q) >> 1ULL;
            
            const bool le = (m_array[m] <= x);
            const size_t le_mask = -size_t(le);
            const size_t gt_mask = ~le_mask;

            if(le) assert(le_mask == SIZE_MAX && gt_mask == 0ULL);
            else   assert(gt_mask == SIZE_MAX && le_mask == 0ULL);
            
            p = (le_mask & m) | (gt_mask & p);
            q = (gt_mask & m) | (le_mask & q);
        }

        const item_t r = m_array[p];
        return Result { r <= x, r == x, r };
    }
};

}

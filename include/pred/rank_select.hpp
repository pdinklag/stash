#pragma once

#include <vec/bit_vector.hpp>
#include <vec/bit_rank.hpp>
#include <vec/bit_select.hpp>

#include <pred/util.hpp>
#include <pred/result.hpp>

namespace pred {

template<typename array_t, typename item_t>
class rank_select {
private:
    size_t      m_num;
    item_t      m_min;
    item_t      m_max;
    bit_vector  m_bv;
    bit_rank    m_rank;
    bit_select1 m_select;

public:
    inline rank_select(const array_t& array)
        : m_num(array.size()),
          m_min(array[0]),
          m_max(array[m_num-1]) {

        assert_sorted_ascending(array);

        m_bv = bit_vector(m_max - m_min);
        for(size_t i = 0; i < m_num; i++) {
            m_bv[array[i] - m_min] = 1;
        }

        m_rank =   bit_rank(m_bv);
        m_select = bit_select1(m_bv);
    }

    inline result<item_t> predecessor(item_t x) const {
        if(unlikely(x < m_min))  return result<item_t> { false, false, x };
        if(unlikely(x >= m_max)) return result<item_t> { true, false, m_max };

        x -= m_min;
        if(m_bv[x]) {
            return result<item_t> { true, true, x }; 
        } else {
            const size_t rank = m_rank(x);
            assert(rank > 0);
            const size_t p = m_select(rank);
            assert(p < x);
            return result<item_t> { true, false, p };
        }
    }
};

}

#pragma once

#include <vec/BitVector.hpp>
#include <vec/Rank.hpp>
#include <vec/Select.hpp>

#include <pred/util.hpp>
#include <pred/Result.hpp>

namespace pred {

template<typename array_t, typename item_t>
class RankSelect {
private:
    size_t    m_num;
    item_t    m_min;
    item_t    m_max;
    BitVector m_bv;
    Rank      m_rank;
    Select1   m_select;

public:
    inline RankSelect(const array_t& array)
        : m_num(array.size()),
          m_min(array[0]),
          m_max(array[m_num-1]) {

        assert_sorted_ascending(array);

        m_bv = BitVector(m_max - m_min);
        for(size_t i = 0; i < m_num; i++) {
            m_bv[array[i] - m_min] = 1;
        }

        m_rank =   Rank(m_bv);
        m_select = Select1(m_bv);
    }

    inline Result<item_t> predecessor(item_t x) const {
        if(__builtin_expect(x < m_min, false))  return Result<item_t> { false, false, x };
        if(__builtin_expect(x >= m_max, false)) return Result<item_t> { true, false, m_max };

        x -= m_min;
        if(m_bv[x]) {
            return Result<item_t> { true, true, x }; 
        } else {
            const size_t rank = m_rank(x);
            assert(rank > 0);
            const size_t p = m_select(rank);
            assert(p < x);
            return Result<item_t> { true, false, p };
        }
    }
};

}

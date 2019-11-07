#pragma once

#include <algorithm>

#include <cmph.h>

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

public:
    inline HashBinSearch(const array_t& array)
        : m_num(array.size()),
          m_min(array[0]),
          m_max(array[m_num-1]),
          m_array(&array) {

        assert_sorted_ascending(array);

        // TODO: build MPHF
    }

    inline Result<item_t> predecessor(const item_t x) const {
        if(__builtin_expect(x < m_min, false))  return Result<item_t> { false, false, x };
        if(__builtin_expect(x >= m_max, false)) return Result<item_t> { true, x == m_max, m_max };

        // TODO
        return Result<item_t> { false, false, x };
    }
};

}

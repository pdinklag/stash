#pragma once

#include <algorithm>

#include <pred/util.hpp>
#include <pred/result.hpp>

#include <pred/binsearch_cache.hpp>

#include <util/math.hpp>

namespace pred {

template<
    typename array_t,
    typename item_t,
    size_t m_alpha,
    size_t m_cache_num = 512ULL / sizeof(item_t)>
class sample {
private:
    const array_t* m_array;
    size_t m_num;
    item_t m_min;
    item_t m_max;

    std::vector<item_t> m_sample;

    using idx_pred_t = binsearch_cache<decltype(m_sample), item_t, m_cache_num>;
    idx_pred_t m_idx;

    using pred_t = binsearch_cache<array_t, item_t, m_cache_num>;
    pred_t m_pred;

public:
    inline sample(const array_t& array)
        : m_num(array.size()),
          m_min(array[0]),
          m_max(array[m_num-1]),
          m_array(&array) {

        assert_sorted_ascending(array);

        // sample
        const size_t num_samples = m_num / m_alpha;

        m_sample.reserve(num_samples);
        for(size_t i = 0; i < num_samples; i++) {
            m_sample.emplace_back(array[i * m_alpha]);
        }

        m_idx = idx_pred_t(m_sample);
        m_pred = pred_t(array);
    }

    inline result predecessor(const item_t x) const {
        if(unlikely(x < m_min))  return result { false, 0 };
        if(unlikely(x >= m_max)) return result { true, m_num-1 };

        auto pred = m_idx.predecessor(x);
        assert(pred.exists);

        const size_t p = pred.pos * m_alpha;
        if(unlikely(x == m_sample[pred.pos])) {
            return result { true, p };
        } else {
            const size_t q = std::min(p + m_alpha, m_num - 1);
            return m_pred.predecessor_seeded(x, p, q);
        }
    }

    inline result successor(const item_t x) const {
        if(unlikely(x <= m_min)) return result { true, 0 };
        if(unlikely(x > m_max))  return result { false, 0 };

        auto succ = m_idx.successor(x);
        assert(succ.exists);

        const size_t q = succ.pos * m_alpha;
        if(unlikely(x == m_sample[succ.pos])) {
            return result { true, q };
        } else {
            const size_t p = (q - m_alpha) & (-(q >= m_alpha)); // std::max(q - m_alpha, 0)
            return m_pred.successor_seeded(x, p, q);
        }
    }
};

}

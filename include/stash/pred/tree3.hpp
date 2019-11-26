#pragma once

#include <stash/pred/result.hpp>
#include <stash/pred/util.hpp>

#include <stash/vec/int_vector.hpp>
#include <stash/util/math.hpp>

#include <vector>

namespace stash {
namespace pred {

template<
    typename array_t,
    typename item_t,
    size_t   m_bits1,
    size_t   m_bits2,
    size_t   m_bits3,
    size_t   m_cache_num = 512ULL / sizeof(item_t)
>
class tree3 {
private:
    static_assert(m_bits1 + m_bits2 + m_bits3 == 8 * sizeof(item_t),
        "the bits on the three levels must sum up to the size of an item");

    static constexpr uint64_t m_mask2 = (1ULL << m_bits2) - 1;
    static constexpr uint64_t m_mask3 = (1ULL << m_bits3) - 1;

    static constexpr uint64_t key1(item_t x) {
        return x >> (m_bits2 + m_bits3);
    }

    static constexpr uint64_t key2(item_t x) {
        return (x >> m_bits3) & m_mask2;
    }

    static constexpr uint64_t key3(item_t x) {
        return x & m_mask3;
    }

    const array_t* m_array;
    size_t m_num;
    item_t m_min;
    item_t m_max;

    struct node {
        size_t     offs;
        int_vector keys;

        inline node() : offs(0) {
        }

        inline node(size_t _offs, size_t size, size_t key_bits)
            : offs(_offs),
              keys(size, key_bits) {

            assert(size > 0);
        }
    };

    node                m_layer1; // one keyset
    std::vector<node>   m_layer2; // offsets and keysets
    std::vector<size_t> m_layer3; // only offsets

public:
    inline tree3(const array_t& array)
        : m_num(array.size()),
          m_min(array[0]),
          m_max(array[m_num-1]),
          m_array(&array) {

        assert_sorted_ascending(array);

        // total numbers of nodes on layers 2 and 3
        size_t num2 = 1;
        size_t num3 = 1;

        // counting scan
        {
            size_t offs2 = 0;
            size_t offs3 = 0;

            uint64_t prev1 = key1(m_min);
            uint64_t prev2 = key2(m_min);
            for(size_t i = 1; i < m_num; i++) {
                const item_t x = (*m_array)[i];
                const uint64_t k1 = key1(x);
                const uint64_t k2 = key2(x);

                const bool new_l2_node = (k1 > prev1);
                const bool new_l3_node = new_l2_node || (k2 > prev2);

                if(new_l2_node) {
                    // new node on layer2
                    ++num2;
                    m_layer2.emplace_back(offs2, i - offs2, m_bits2);
                    offs2 = i;
                    prev1 = k1;
                }
                
                if(new_l3_node) {
                    // new node on layer 3
                    ++num3;
                    m_layer3.emplace_back(offs3);
                    offs3 = i;
                    prev2 = k2;
                }
            }

            // final nodes on layer 2 and 3
            m_layer2.emplace_back(offs2, m_num - offs2, m_bits2);
            m_layer3.emplace_back(offs3);

            // compress
            m_layer2.shrink_to_fit();
            m_layer3.shrink_to_fit();
        }

        // fill keys on layer 1 and layer 2
        {
            uint64_t prev1 = key1(m_min);
            size_t cur_key1  = 0;
            size_t cur_node2 = 0;
            size_t cur_key2  = 0;

            m_layer1 = node(0, num2, m_bits1);
            m_layer1.keys[cur_key1++] = prev1;
            m_layer2[cur_node2].keys[cur_key2++] = key2(m_min);
            
            for(size_t i = 1; i < m_num; i++) {
                const item_t x = (*m_array)[i];
                const uint64_t k1 = key1(x);
                const uint64_t k2 = key2(x);

                if(k1 > prev1) {
                    ++cur_node2;
                    cur_key2 = 0;
                    m_layer1.keys[cur_key1++] = k1;
                    prev1 = k1;
                }

                m_layer2[cur_node2].keys[cur_key2++] = k2;
            }
        }

        //
        std::cout << "num2=" << num2 << ", num3=" << num3 << std::endl;
    }

    inline result predecessor(const item_t x) const {
        if(unlikely(x < m_min))  return result { false, 0 };
        if(unlikely(x >= m_max)) return result { true, m_num-1 };

        // TODO: implement
        const uint64_t k1 = key1(x);
        // TODO: find predecessor (pred1) of k1 in m_layer1.keys
        // TODO: find predecessor (pred2) of k2 in m_layer2[pred1].keys
        // -> if it doesn't exist, the result is m_layer2[pred1].offs - 1
        // TODO: find predecessor of x in interval [m_layer3[pred2].offs, m_layer3[pred2+1].offs]
        
        const uint64_t k2 = key2(x);
        
        return result { false, 0 };
    }
};

}}

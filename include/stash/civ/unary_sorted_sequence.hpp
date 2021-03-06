#pragma once

#include <memory>
#include <utility>

#include <stash/vec/bit_vector.hpp>
#include <stash/vec/bit_rank.hpp>
#include <stash/vec/bit_select.hpp>
#include <stash/util/assert.hpp>

namespace stash {
namespace civ {

class unary_sorted_sequence {
private:
    size_t                      m_size;
    std::shared_ptr<bit_vector> m_bits;
    bit_rank                    m_rank;
    bit_select0                 m_sel0;

    template<typename T>
    size_t encode_unary(size_t pos, T value) {
        auto& bits = *m_bits;
        while(value) {
            bits[pos++] = 1;
            --value;
        }
        bits[pos++] = 0;
        return pos;
    }

public:
    inline unary_sorted_sequence() : m_size(0) {
    }

    inline unary_sorted_sequence(const int_vector& other) {
        *this = other;
    }

    inline unary_sorted_sequence(int_vector&& other) {
        *this = std::move(other);
    }

    template<typename array_t>
    inline unary_sorted_sequence(const array_t& array) : m_size(array.size()) {
        assert_sorted_ascending(array);

        // unary encoding
        {
            const size_t max = size_t(array[m_size-1]);
            const size_t num_bits = m_size + max;
            m_bits = std::make_shared<bit_vector>(num_bits);
            
            auto prev = array[0];
            size_t pos = encode_unary(0, prev);
            for(size_t i = 1; i < m_size; i++) {
                const auto v = array[i];
                pos = encode_unary(pos, v - prev);
                prev = v;
            }

            assert(pos == num_bits);
        }
        
        // rank + select
        m_rank = bit_rank(m_bits);
        m_sel0 = bit_select0(m_bits);
    }

    inline unary_sorted_sequence& operator=(const unary_sorted_sequence& other) {
        m_size = other.m_size;
        m_bits = other.m_bits;
        m_rank = other.m_rank;
        m_sel0 = other.m_sel0;
        return *this;
    }

    inline unary_sorted_sequence& operator=(unary_sorted_sequence&& other) {
        m_size = other.m_size;
        m_bits = std::move(other.m_bits);
        m_rank = std::move(other.m_rank);
        m_sel0 = std::move(other.m_sel0);
        return *this;
    }

    inline uint64_t operator[](size_t i) const {
        assert(i < m_size);
        return m_rank(m_sel0(i+1));
    }

    inline size_t size() const {
        return m_size;
    }
};

}}

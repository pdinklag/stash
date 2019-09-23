#pragma once

#include <cstdint>
#include <vector>

class bit_vector {
private:
    size_t m_size;
    std::vector<uint64_t> m_bits;
    
    inline static size_t block(size_t i) {
        return i >> 6ULL; // divide by 64
    }

    inline static size_t offset(size_t i) {
        return i & 63ULL; // mod 64
    }

public:
    inline bit_vector(size_t size) : m_size(size) {
        /* BVCC - Allokation */
        const size_t q = block(size);
        const size_t k = offset(size);
        m_bits.resize(k ? q+1 : q);
    }

    inline uint64_t block64(size_t i) const {
        return m_bits[i];
    }

    inline size_t num_blocks() const {
        return m_bits.size();
    }

    inline void bitset(size_t i, bool b) {
        /* BVCC - Bit an Position i auf b setzen */
        const size_t q = block(i);
        const size_t k = offset(i);
        const size_t mask = (1ULL << k);

        // TODO: avoid branch!
        if(b) {
            m_bits[q] |= mask;
        } else {
            m_bits[q] &= ~mask;
        }
    }

    inline bool bitread(size_t i) const {
        /* BVCC - Bit aus Position i lesen */
        const size_t q = block(i);
        const size_t k = offset(i);
        const size_t mask = (1ULL << k);
        return bool(m_bits[q] & mask);
    }

    inline size_t size() const {
        return m_size;
    }
};

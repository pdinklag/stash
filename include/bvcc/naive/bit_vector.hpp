#pragma once

#include <vector>

class bit_vector {
private:
    size_t m_size;
    std::vector<bool> m_bv; /* BVCC - Ersetzen! */

public:
    inline bit_vector(size_t size) : m_size(size) {
        /* BVCC - Allokation */
        m_bv = std::vector<bool>(size);
    }

    inline void bitset(size_t i, bool b) {
        /* BVCC - Bit an Position i auf b setzen */
        m_bv[i] = b;
    }

    inline bool bitread(size_t i) const {
        /* BVCC - Bit aus Position i lesen */
        return m_bv[i];
    }

    inline size_t size() const {
        return m_size;
    }
};

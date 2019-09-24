#pragma once

#include <vector>
#include "bit_vector.hpp"

class bit_vector_rank {
private:
    const bit_vector* m_bv;

    std::vector<size_t> m_rank0, m_rank1; /* BVCC - Ersetzen! */

public:
    inline bit_vector_rank(const bit_vector& bv) : m_bv(&bv) {
        /* BVCC - Konstruktion */
        const size_t n = m_bv->size();
        m_rank0 = std::vector<size_t>(n);
        m_rank1 = std::vector<size_t>(n);

        size_t r0 = 0, r1 = 0;
        for(size_t i = 0; i < n; i++) {
            if(m_bv->bitread(i)) {
                ++r1;
            } else {
                ++r0;
            }

            m_rank0[i] = r0;
            m_rank1[i] = r1;
        }
    }

    inline size_t rank1(size_t i) const {
        return m_rank1[i]; /* BVCC - rank1(i) */
    }

    inline size_t rank0(size_t i) const {
        return m_rank0[i]; /* BVCC - rank0(i) */
    }
};

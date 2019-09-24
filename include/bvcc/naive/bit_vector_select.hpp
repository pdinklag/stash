#pragma once

#include <vector>
#include "bit_vector.hpp"

class bit_vector_select {
private:
    const bit_vector* m_bv;

    std::vector<size_t> m_select0, m_select1; /* BVCC - Ersetzen! */

public:
    inline bit_vector_select(const bit_vector& bv) : m_bv(&bv) {
        /* BVCC - Konstruktion */
        for(size_t i = 0; i < m_bv->size(); i++) {
            if(m_bv->bitread(i)) {
                m_select1.push_back(i);
            } else {
                m_select0.push_back(i);
            }
        }
    }

    inline size_t select1(size_t k) const {
        /* BVCC - select1(k) */
        --k;
        return (k < m_select1.size()) ? m_select1[k] : m_bv->size();
    }

    inline size_t select0(size_t k) const {
        /* BVCC - select0(k) */
        --k;
        return (k < m_select0.size()) ? m_select0[k] : m_bv->size();
    }
};

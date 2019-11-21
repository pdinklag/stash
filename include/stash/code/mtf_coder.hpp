#pragma once

#include <vector>
#include <stash/code/coder.hpp>

namespace stash {
namespace code {

template<typename coder_t, typename sym_coder_t, typename num_coder_t>
class mtf_coder : public coder {
private:
    static constexpr size_t MAX_SYMS = 256ULL;

    size_t  m_sigma;
    uint8_t m_syms[MAX_SYMS];
    coder_t m_coder;

public:
    inline mtf_coder(const std::string s, bit_ostream& out) {
        std::vector<bool> occ(MAX_SYMS);

        m_sigma = 0;
        for(uint8_t c : s) {
            if(!occ[c]) {
                // add new symbol to list
                m_syms[m_sigma++] = c;
            }
            occ[c] = 1;
        }

        sym_coder_t sym_coder;
        num_coder_t num_coder;

        num_coder.encode(out, m_sigma);
        for(size_t x = 0; x < m_sigma; x++) {
            sym_coder.encode(out, m_syms[x]);
        }
    }

    inline mtf_coder(bit_istream& in) {
        sym_coder_t sym_coder;
        num_coder_t num_coder;
        
        m_sigma = num_coder.template decode<>(in);
        for(size_t x = 0; x < m_sigma; x++) {
            m_syms[x] = sym_coder.template decode<uint8_t>(in);
        }
    }

    inline bool eof(bit_istream& in) {
        return in.eof();
    }

    inline void encode(bit_ostream& out, uint8_t c) {
        // look for c
        for(size_t x = 0; x < m_sigma; x++) {
            if(m_syms[x] == c) {
                m_coder.encode(out, x);
                break;
            }
        }
        update(c);
    }
    
    inline uint8_t decode(bit_istream& in) {
        uint8_t c = m_syms[m_coder.template decode<uint8_t>(in)];
        update(c);
        return c;
    }

private:
    inline void update(uint8_t c) {
        // find c
        size_t rank = SIZE_MAX;
        for(size_t x = 0; x < m_sigma; x++) {
            if(m_syms[x] == c) {
                rank = x;
                break;
            }
        }
        
        // move to front
        for(size_t x = rank; x > 0; x--) {
            m_syms[x] = m_syms[x-1];
        }
        m_syms[0] = c;
    }
};

}}

#pragma once

#include <code/coder.hpp>

template<size_t m_bits = 64>
class binary_coder : public coder {
public:
    using coder::coder;

    template<typename T>
    void encode(bit_ostream& out, T value) {
        out.write_binary(value, m_bits);
    }
    
    template<typename T>
    void encode(bit_ostream& out, T value, size_t bits) {
        out.write_binary(value, bits);
    }

    template<typename T = uint64_t>
    T decode(bit_istream& in) {
        return in.template read_binary<T>(m_bits);
    }

    template<typename T = uint64_t>
    T decode(bit_istream& in, size_t bits) {
        return in.template read_binary<T>(bits);
    }
};

#pragma once

#include <code/CoderBase.hpp>

template<size_t m_bits = 64>
class BinaryCoder : public CoderBase {
public:
    using CoderBase::CoderBase;

    template<typename T>
    void encode(BitOStream& out, T value) {
        out.write_binary(value, m_bits);
    }
    
    template<typename T>
    void encode(BitOStream& out, T value, size_t bits) {
        out.write_binary(value, bits);
    }

    template<typename T = uint64_t>
    T decode(BitIStream& in) {
        return in.template read_binary<T>(m_bits);
    }

    template<typename T = uint64_t>
    T decode(BitIStream& in, size_t bits) {
        return in.template read_binary<T>(bits);
    }
};

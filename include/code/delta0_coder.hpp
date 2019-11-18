#pragma once

#include <code/coder.hpp>

class delta0_coder : public coder {
public:
    using coder::coder;

    template<typename T>
    void encode(bit_ostream& out, T value) {
        out.write_delta(value + T(1));
    }

    template<typename T = uint64_t>
    T decode(bit_istream& in) {
        return in.template read_delta<T>() - T(1);
    }
};

#pragma once

#include <io/BitIStream.hpp>
#include <io/BitOStream.hpp>

class DeltaCoder {
public:
    inline DeltaCoder() {
    }

    template<typename T>
    void encode(BitOStream& out, T value) {
        out.write_delta(value);
    }

    template<typename T = uint64_t>
    T decode(BitIStream& in) {
        return in.template read_delta<T>();
    }
};

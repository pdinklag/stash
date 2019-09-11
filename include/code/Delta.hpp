#pragma once

#include <code/CoderBase.hpp>

class DeltaCoder : public CoderBase {
public:
    using CoderBase::CoderBase;

    template<typename T>
    void encode(BitOStream& out, T value) {
        out.write_delta(value);
    }

    template<typename T = uint64_t>
    T decode(BitIStream& in) {
        return in.template read_delta<T>();
    }
};

#pragma once

#include <io/BitIStream.hpp>
#include <io/BitOStream.hpp>

class CoderBase {
public:
    inline CoderBase() {
    }

    inline CoderBase(const std::string& s, BitOStream& out) : CoderBase() {
    }

    inline CoderBase(BitIStream& in) : CoderBase() {
    }

    inline bool eof(BitIStream& in) {
        return in.eof();
    }
};

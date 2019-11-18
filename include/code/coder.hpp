#pragma once

#include <io/bit_istream.hpp>
#include <io/bit_ostream.hpp>

class coder {
public:
    inline coder() {
    }

    inline coder(const std::string& s, bit_ostream& out) : coder() {
    }

    inline coder(bit_istream& in) : coder() {
    }

    inline bool eof(bit_istream& in) {
        return in.eof();
    }
};

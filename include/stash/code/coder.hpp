#pragma once

#include <stash/io/bit_istream.hpp>
#include <stash/io/bit_ostream.hpp>

namespace stash {
namespace code {

using io::bit_istream;
using io::bit_ostream;

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

}}

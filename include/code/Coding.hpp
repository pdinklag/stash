#pragma once

#include <sstream>

template<typename coder_t>
inline void encode(const std::string& s, BitOStream& out) {
    coder_t coder(s, out);
    for(uint8_t c : s) {
        coder.encode(out, c);
    }
}

template<typename coder_t>
inline std::string decode(BitIStream& in) {
    std::ostringstream s;
    coder_t coder(in);
    while(!coder.eof(in)) {
        s << uint8_t(coder.decode(in));
    }
    return s.str();
}

#pragma once

#include <cassert>
#include <climits>
#include <iostream>
#include <util/math.hpp>

/// \brief Wrapper for output streams that provides bitwise writing
/// functionality.
///
/// Bits are written into a buffer byte, which is written to the output when
/// it is either filled or when a flush is explicitly requested.
class bit_ostream {
    std::ostream* m_stream;

    uint8_t m_next;
    int8_t m_cursor;
    static constexpr int8_t MSB = 7;

    size_t m_bits_written = 0;

    inline bool is_dirty() const {
        return m_cursor != MSB;
    }

    inline void reset() {
        m_next = 0;
        m_cursor = MSB;
    }

    inline void write_next() {
        m_stream->put(char(m_next));
        reset();
    }

public:
    inline bit_ostream(std::ostream& stream) : m_stream(&stream) {
        reset();
    }

    inline ~bit_ostream() {
        uint8_t set_bits = MSB - m_cursor; // will only be in range 0 to 7
        if(m_cursor >= 2) {
            // if there are at least 3 bits free in the byte buffer,
            // write them into the cursor at the last 3 bit positions
            m_next |= set_bits;
            write_next();
        } else {
            // else write out the byte, and write the length into the
            // last 3 bit positions of the next byte
            write_next();
            m_next = set_bits;
            write_next();
        }
    }

    inline std::ostream& stream() {
        return *m_stream;
    }

    inline void write_bit(bool set) {
        if (set) {
            m_next |= (1 << m_cursor);
        }

        m_cursor--;

        if (m_cursor < 0) {
            write_next();
        }

        ++m_bits_written;
    }

    template<class T>
    inline void write_binary(const T value, size_t bits = sizeof(T) * CHAR_BIT) {
        assert(bits <= 64ULL);
        assert(m_cursor >= 0);
        const size_t bits_left_in_next = size_t(m_cursor + 1);
        assert(bits_left_in_next <= 8ULL);

        if(bits < bits_left_in_next) {
            // we are writing only few bits
            // simply use the bit-by-bit method
            for (int i = bits - 1; i >= 0; i--) {
                write_bit((value & T(T(1) << i)) != T(0));
            }
        } else {
            // we are at least finishing the next byte
            const size_t in_bits = bits;

            // mask low bits of value
            size_t v = (bits < 64ULL) ?
                (size_t(value) & ((1ULL << bits) - 1ULL)) : size_t(value);

            // fill it up next byte and continue with remaining bits
            bits -= bits_left_in_next;
            m_next |= (v >> bits);
            write_next();

            v &= (1ULL << bits) - 1ULL; // mask remaining bits

            // write as many full bytes as possible
            if(bits >= 8ULL) {
                const size_t n = bits / 8ULL;
                bits %= 8ULL;

                // convert full bytes into BIG ENDIAN (!) representation
                #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
                    const size_t v_bytes = __builtin_bswap64(v >> bits);
                #else
                    const size_t v_bytes = v >> bits;
                #endif

                const size_t off  = sizeof(size_t) - n;
                m_stream->write(((const char*)&v_bytes) + off, n);

                v &= (1ULL << bits) - 1ULL; // mask remaining bits
            }

            if(bits) {
                // write remaining bits
                // they (must) fit into the next byte, so just stuff them in
                assert(bits < 8ULL);
                assert(v < 256ULL);
                m_next = (v << (8ULL - bits));
                m_cursor = MSB - int8_t(bits);
                assert(m_cursor >= 0);
            }

            m_bits_written += in_bits;
        }
    }

    template<typename T>
    inline void write_unary(T value) {
        while(value--) {
            write_bit(0);
        }
        write_bit(1);
    }

    template<typename T>
    inline void write_gamma(T value) {
        assert(value > T(0));

        const auto m = log2_floor(value);
        write_unary(m);
        if(m > 0) write_binary(value, m); // cut off leading 1
    }

    template<typename T>
    inline void write_delta(T value) {
        assert(value > T(0));

        auto m = log2_floor(value);
        write_gamma(m+1); // cannot encode zero
        if(m > 0) write_binary(value, m); // cut off leading 1
    }

    template<typename T>
    inline void write_rice(T value, uint8_t p) {
        const uint64_t q = uint64_t(value) >> p;

        write_gamma(q + 1);
        write_binary(value, p); // r is exactly the lowest p bits of v
    }

    inline size_t bits_written() const { return m_bits_written; }
};

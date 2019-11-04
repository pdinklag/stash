#pragma once

#include <cstdint>

// creates a mask consisting of the given number of bits
inline constexpr uint64_t bit_mask(uint64_t bits) {
    return (1ULL << bits) - 1ULL;
}

// integer division rounding up
inline constexpr uint64_t div_ceil(uint64_t a, uint64_t b) {
    const uint64_t q = a / b;
    return (a % b == 0) ? q : q + 1;
}

inline constexpr uint64_t log2_ceil(uint64_t x) {
    return 64ULL - __builtin_clzll(x);
} 

// rank on a 64-bit value
inline constexpr uint8_t rank1_u64(uint64_t v) {
    return __builtin_popcountll(v);
}

// rank on a 64-bit value up to bit x
inline constexpr uint8_t rank1_u64(uint64_t v, uint8_t x) {
    return __builtin_popcountll(v & (UINT64_MAX >> (~x & 63ULL))); // 63 - x
}

// rank on a 64-bit value between a and b
inline constexpr uint8_t rank1_u64(uint64_t v, uint8_t a, uint8_t b) {
    const uint64_t mask_a = UINT64_MAX << a;
    const uint64_t mask_b = UINT64_MAX >> (~b & 63ULL); // 63 - b
    return __builtin_popcountll(v & mask_a & mask_b);
}

/// \brief Returned by \ref select0 and \ref select1 in case the searched
///        bit does not exist in the given input value.
constexpr uint8_t SELECT_FAIL = 0xFF;

/// \brief Finds the position of the k-th 1-bit in the binary representation
///        of the given value.
///
/// The search starts with the least significant bit.
///
/// \param v the input value
/// \param k the searched 1-bit
/// \return the position of the k-th 1-bit (LSBF and zero-based),
///         or \ref SELECT_FAIL if no such bit exists
inline constexpr uint8_t select1_u64(uint64_t v, uint8_t k) {
    uint8_t pos = 0;
    while(v && k && pos < 64) {
        const size_t z = __builtin_ctzll(v)+1;
        pos += z;
        v >>= z;
        --k;
    }
    return k ? SELECT_FAIL : pos - 1;
}

/// \brief Finds the position of the k-th 1-bit in the binary representation
///        of the given value.
///
/// \param v the input value
/// \param l the bit (LSBF order) to start searching from
/// \param k the searched 1-bit
/// \return the position of the k-th 1-bit (LSBF and zero-based),
///         or \ref SELECT_FAIL if no such bit exists
inline constexpr uint8_t select1_u64(uint64_t v, uint8_t l, uint8_t k) {
    const uint8_t pos = select1_u64(v >> l, k);
    return (pos != SELECT_FAIL) ? (l + pos) : SELECT_FAIL;
}

/// \brief Finds the position of the k-th 0-bit in the binary representation
///        of the given value.
///
/// The search starts with the least significant bit.
///
/// \param v the input value
/// \param k the searched 0-bit
/// \return the position of the k-th 0-bit (LSBF and zero-based),
///         or \ref SELECT_FAIL if no such bit exists
inline constexpr uint8_t select0_u64(uint64_t v, uint8_t k) {
    return select1_u64(~v, k);
}

/// \brief Finds the position of the k-th 0-bit in the binary representation
///        of the given value.
///
/// \param v the input value
/// \param l the bit (LSBF order) to start searching from
/// \param k the searched 0-bit
/// \return the position of the k-th 0-bit (LSBF and zero-based),
///         or \ref SELECT_FAIL if no such bit exists
inline constexpr uint8_t select0_u64(uint64_t v, uint8_t l, uint8_t k) {
    return select1_u64(~v, l, k);
}

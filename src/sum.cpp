#include <cstdint>
#include <iostream>
#include <vector>

#include <immintrin.h>

#include <stash/io/load_file.hpp>
#include <tlx/cmdline_parser.hpp>

#ifndef __AVX512F__
#pragma message("AVX-512F not supported by CPU")
#endif

uint64_t sum_onebyone(const std::vector<uint64_t>& a) {
    const size_t n = a.size();
    uint64_t sum = 0;
    for(size_t i = 0; i < a.size(); i++) {
        sum += a[i];
    }
    return sum;
}

#ifdef __AVX512F__
uint64_t sum_avx512(const std::vector<uint64_t>& a) {
    const size_t n = a.size();

    uint64_t sum = 0;
    size_t i = 0;
    for(; i+8 < n; i += 8) {
        auto v = _mm512_set_epi64(a[i], a[i+1], a[i+2], a[i+3], a[i+4], a[i+5], a[i+6], a[i+7]);
        sum += _mm512_reduce_add_epi64(v);
    }
    for(; i < n; i++) {
        sum += a[i];
    }
    return sum;
}
#endif

int main(int argc, char** argv) {
    tlx::CmdlineParser cp;

    std::string filename;
    cp.add_param_string("file", filename, "the input filename");
    
    if (!cp.process(argc, argv)) {
        return -1;
    }

    auto a = stash::io::load_file_lines_as_vector<uint64_t>(filename);

    // one by one
    {
        auto sum = sum_onebyone(a);
        std::cout << "sum_onebyone: " << sum << std::endl;
    }

    // AVX512
    #ifdef __AVX512F__
    {
        auto sum = sum_avx512(a);
        std::cout << "sum_avx512: " << sum << std::endl;
    }
    #endif

    return 0;
}

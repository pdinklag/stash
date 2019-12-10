#pragma once

#include <array>
#include <cstdlib>
#include <fstream>
#include <string>
#include <vector>

namespace stash {
namespace io {

inline std::string load_file_as_string(const std::string& filename) {
    std::string s;
    std::ifstream f(filename);
    f.seekg(0, std::ios::end);   
    s.reserve(f.tellg());
    f.seekg(0, std::ios::beg);
    s.assign((std::istreambuf_iterator<char>(f)),
              std::istreambuf_iterator<char>());
    return s;
}

template<typename I, typename O = I>
inline std::vector<O> load_file_as_vector(
    const std::string& filename,
    const size_t bufsize = 16384ULL) {

    std::vector<O> v;

    std::ifstream f(filename);
    f.seekg(0, std::ios::end);   
    const size_t sz = f.tellg();
    f.seekg(0, std::ios::beg);

    const size_t total = sz / sizeof(I);
    v.reserve(total);

    I* buffer = new I[bufsize];
    
    size_t left = total;
    while(left) {
        const size_t num = std::min(bufsize, left);
        f.read((char*)buffer, num * sizeof(I));

        for(size_t i = 0; i < num; i++) {
            v.push_back(O(buffer[i]));
        }

        left -= num;
    }
    
    delete[] buffer;
    return v;
}

template<typename O>
inline std::vector<O> load_file_lines_as_vector(const std::string& filename) {
    std::vector<O> v;
    std::ifstream f(filename);

    // 2^64 has 20 decimal digits, so a buffer of size 24 should be safe
    for(std::array<char, 24> linebuf; f.getline(&linebuf[0], 24);) {
        if(linebuf[0]) {
            v.push_back(std::atoll(&linebuf[0]));
        }
    }

    return v;
}

}}

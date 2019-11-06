#pragma once

#include <fstream>
#include <string>
#include <vector>

std::string load_file_as_string(const std::string& filename) {
    std::string s;
    std::ifstream f(filename);
    f.seekg(0, std::ios::end);   
    s.reserve(f.tellg());
    f.seekg(0, std::ios::beg);
    s.assign((std::istreambuf_iterator<char>(f)),
              std::istreambuf_iterator<char>());
    return s;
}

template<typename T>
std::vector<T> load_file_as_vector(
    const std::string& filename,
    const size_t bufsize = 16384ULL) {

    std::vector<T> v;

    std::ifstream f(filename);
    f.seekg(0, std::ios::end);   
    const size_t sz = f.tellg();
    f.seekg(0, std::ios::beg);

    const size_t total = sz / sizeof(T);
    v.reserve(total);

    T* buffer = new T[bufsize];
    
    size_t left = total;
    while(left) {
        const size_t num = std::min(bufsize, left);
        f.read((char*)buffer, num * sizeof(T));

        for(size_t i = 0; i < num; i++) {
            v.push_back(buffer[i]);
        }

        left -= num;
    }
    
    delete[] buffer;
    return v;
}

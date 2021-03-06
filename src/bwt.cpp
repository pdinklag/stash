#include <iostream>
#include <fstream>
#include <vector>

#include <stash/util/file_size.hpp>

#include <divsufsort64.h>
#include <tlx/cmdline_parser.hpp>

int main(int argc, char** argv) {
    // params
    std::string input_filename; // required
    sauchar_t sentinel;

    {
        std::string sentinel_str("\0", 1);
        
        tlx::CmdlineParser cp;
        cp.add_param_string("file", input_filename, "The input file.");
        cp.add_string('s', "sentinel", sentinel_str, "How to print the sentinel character.");
        
        if (!cp.process(argc, argv)) {
            return -1;
        }

        sentinel = sentinel_str[0];
    }

    // read input file
    const size_t input_filesize = stash::file_size(input_filename);
    const size_t n = input_filesize + 1;

    sauchar_t* input = new sauchar_t[n];
    {
        std::ifstream f(input_filename);
        f.read((char*)input, input_filesize);

        // verify
        for(size_t i = 0; i < input_filesize; i++) {
            if(input[i] == sentinel) {
                std::cerr << "input must not contain sentinel" << std::endl;
                return -2;
            }
        }
    }
    input[n-1] = 0; // sentinel

    // construct suffix array
    saidx64_t* sa = new saidx64_t[n];
    divsufsort64(input, sa, n);

    // print BWT
    for(size_t i = 0; i < n; i++) {
        const auto s = sa[i];
        const auto c = input[s ? s-1 : n-1];
        std::cout << (c ? c : sentinel);
    }

    // clean up
    delete[] sa;
    delete[] input;
}

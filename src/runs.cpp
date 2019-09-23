#include <algorithm>
#include <iostream>
#include <fstream>

#include <tlx/cmdline_parser.hpp>

#include <io/BitOStream.hpp>
#include <util/FileSize.hpp>

int main(int argc, char** argv) {
    // params
    std::string input_filename; // required

    std::string pos_filename;
    std::string bv_filename;

    size_t bufsize = 16'777'216ULL; // 16 Mi

    {
        tlx::CmdlineParser cp;
        cp.add_param_string("file", input_filename, "The input file.");

        cp.add_param_string("pos", pos_filename, "The output filename for run starting positions.");
        cp.add_param_string("bv", bv_filename, "The output filename for the run bit vector.");

        cp.add_bytes("bs", bufsize, "Read buffer size.");
        
        if (!cp.process(argc, argv)) {
            return -1;
        }
    }

    // process
    const size_t n = file_size(input_filename);
    size_t runs = 0;

    {
        std::ifstream ins(input_filename);
        
        std::ofstream pos(pos_filename);
        std::ofstream bvs(bv_filename);
        BitOStream bv(bvs);

        char* buf = new char[bufsize];
        size_t p = 0;

        char last;

        while(p < n) {
            const size_t num = std::min(bufsize, n-p);
            ins.read(buf, num);
            
            for(size_t i = 0; i < num; i++) {
                const char c = buf[i];
                if(p+i == 0 || c != last) {
                    // position i is the start of a BWT run
                    const size_t run = p + i;
                    ++runs;
                    
                    pos.write((const char*)&run, sizeof(size_t));
                    bv.write_bit(1);

                    last = c;
                } else {
                    bv.write_bit(0);
                }
            }

            p += num;
        }

        delete[] buf;
    }

    std::cout << "Extracted " << runs << " runs." << std::endl;
}

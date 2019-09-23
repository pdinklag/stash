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

    {
        tlx::CmdlineParser cp;
        cp.add_param_string("file", input_filename, "The input file.");

        cp.add_param_string("pos", pos_filename, "The output filename for run starting positions.");
        cp.add_param_string("bv", bv_filename, "The output filename for the run bit vector.");
        
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

        char c, last;
        for(size_t i = 0; i < n; i++) {
            ins.get(c);
            if(i == 0 || c != last) {
                // position i is the start of a BWT run
                ++runs;
                pos.write((const char*)&i, sizeof(size_t));
                bv.write_bit(1);

                last = c;
            } else {
                bv.write_bit(0);
            }
        }
    }

    std::cout << "Extracted " << runs << " runs." << std::endl;
}

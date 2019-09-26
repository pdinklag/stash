#include <algorithm>
#include <iostream>
#include <fstream>
#include <memory>

#include <tlx/cmdline_parser.hpp>

#include <io/BitOStream.hpp>
#include <util/FileSize.hpp>

int main(int argc, char** argv) {
    // params
    std::string input_filename; // required

    std::string pos_filename;
    std::string bv_filename;

    size_t bufsize = 16'777'216ULL; // 16 Mi
    size_t wbufsize = 2'097'152;
    size_t bytes_per_pos = sizeof(size_t); // 8 bytes = 64 bits

    {
        tlx::CmdlineParser cp;
        cp.add_param_string("file", input_filename, "The input file.");

        cp.add_string("out-pos", pos_filename, "The output filename for run starting positions.");
        cp.add_string("out-bv", bv_filename, "The output filename for the run bit vector.");

        cp.add_bytes("rbufsize", bufsize, "Read buffer size (bytes).");
        cp.add_bytes("wbufsize", wbufsize, "Write buffer size (words).");
        cp.add_bytes('w', "pos-width", bytes_per_pos, "Position type width - default 8 bytes.");
        
        if (!cp.process(argc, argv)) {
            return -1;
        }

        if(pos_filename.empty() && bv_filename.empty()) {
            std::cerr << "no output specified" << std::endl;
            return -1;
        }
    }

    // process
    const size_t n = file_size(input_filename);
    size_t runs = 0;

    {
        std::ifstream ins(input_filename);

        std::unique_ptr<std::ofstream> pos;
        char* wbuf = nullptr;
        if(!pos_filename.empty()) {
            pos = std::make_unique<std::ofstream>(pos_filename);
            wbuf = new char[wbufsize * bytes_per_pos + sizeof(size_t)];
        }

        std::unique_ptr<std::ofstream> bvs;
        std::unique_ptr<BitOStream> bv;
        if(!bv_filename.empty()) {
            bvs = std::make_unique<std::ofstream>(bv_filename);
            bv = std::make_unique<BitOStream>(*bvs);
        }

        char* buf = new char[bufsize];
        size_t p = 0;

        size_t wc = 0;
        char* wp = wbuf;

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
                    
                    if(pos) {
                        *((size_t*)wp) = run;
                        wp += bytes_per_pos;
                        if(++wc >= wbufsize) {
                            pos->write(wbuf, wbufsize * bytes_per_pos);
                            wp = wbuf;
                            wc = 0;
                        }
                    }
                    if(bv) bv->write_bit(1);

                    last = c;
                } else {
                    if(bv) bv->write_bit(0);
                }
            }

            p += num;
        }

        if(wc > 0) {
            pos->write(wbuf, wc * bytes_per_pos);
        }

        if(wbuf) delete[] wbuf;
        delete[] buf;
    }

    std::cout << "Extracted " << runs << " runs." << std::endl;
}

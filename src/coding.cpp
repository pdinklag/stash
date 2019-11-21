#include <iostream>
#include <fstream>
#include <streambuf>
#include <sstream>

#include <tlx/cmdline_parser.hpp>

#include <stash/code/ascii_coder.hpp>
#include <stash/code/binary_coder.hpp>
#include <stash/code/delta_coder.hpp>
#include <stash/code/delta0_coder.hpp>
#include <stash/code/mtf_coder.hpp>
#include <stash/code/coding.hpp>

#include <stash/huff/huffman_coder.hpp>
#include <stash/huff/knuth_coder.hpp>
#include <stash/huff/forward_coder.hpp>
#include <stash/huff/hybrid_coder.hpp>

#include <stash/io/load_file.hpp>
#include <stash/util/time.hpp>

using namespace stash;
using namespace stash::code;

using mtf_coder_t = mtf_coder<ascii_coder, ascii_coder, binary_coder<>>;

template<typename coder_t>
void test(const std::string& input, const std::string& filename, bool verify) {
    std::cout << "# " << filename << " ..." << std::endl;
    
    // encode using MTF first
    uint64_t encode_time;
    size_t bits_written;
    {
        std::string mtf_code;
        {
            std::ostringstream mtf_ss;
            {
                bit_ostream out(mtf_ss);
                std::cout << "# encoding MTF ..." << std::endl;
                
                encode<mtf_coder_t>(input, out);
                bits_written = out.bits_written();
            }
            mtf_code = mtf_ss.str();
        }
        
        // encode using given coder
        auto t0 = time();
        {
            std::ofstream f(filename);
            bit_ostream out(f);

            std::cout << "# encoding " << filename << "..." << std::endl;
            encode<coder_t>(mtf_code, out);
            bits_written = out.bits_written();
        }
        encode_time = time() - t0;
    }

    bool success;
    if(verify) {
        // decode using given coder
        {
            std::string mtf_dec;
            {
                std::ifstream f(filename);
                bit_istream in(f);

                std::cout << "# decoding " << filename << " ..." << std::endl;
                mtf_dec = decode<coder_t>(in);
            }

            // decode MTF
            std::string dec;
            {
                std::istringstream ss(mtf_dec);
                bit_istream in(ss);

                std::cout << "# decoding MTF ..." << std::endl;
                dec = decode<mtf_coder_t>(in);
            }

            std::cout << "# checking ..." << std::endl;
            success = (dec == input);
        }
    } else {
        success = true;
    }

    std::cout << "RESULT algo=" << filename
        << ", in=" << 8 * input.length()
        << ", out=" << bits_written
        << ", time=" << encode_time
        << ", rate=" << double(bits_written) / double(8 * input.length())
        << ", success=" << success
        << std::endl;
}

int main(int argc, char** argv) {
    tlx::CmdlineParser cp;

    std::string input_filename;
    cp.add_param_string("file", input_filename, "The input file.");

    std::string outfile_prefix = "";
    cp.add_string('d', "dir", outfile_prefix, "The output directory.");

    bool verify = false;
    cp.add_flag("verify", verify, "Decode and verify.");

    if (!cp.process(argc, argv)) {
        return -1;
    }
    
    auto text = io::load_file_as_string(input_filename);

    //test<ascii_coder>(text, outfile_prefix + "ascii", verify);
    test<huff::huffman_coder<ascii_coder, delta_coder>>(text, outfile_prefix + "huffman", verify);
    //test<huff::knuth_coder<ascii_coder>>(text, outfile_prefix + "knuth", verify);
    test<huff::forward_coder<ascii_coder, delta_coder>>(text, outfile_prefix + "forward", verify);
    test<huff::hybrid_coder<ascii_coder, delta_coder>>(text, outfile_prefix + "hybrid", verify);
}

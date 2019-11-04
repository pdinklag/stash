#include <iostream>
#include <fstream>
#include <streambuf>
#include <sstream>

#include <tlx/cmdline_parser.hpp>

#include <code/ASCII.hpp>
#include <code/Binary.hpp>
#include <code/Delta.hpp>
#include <code/Delta0.hpp>
#include <code/MTF.hpp>
#include <code/Coding.hpp>

#include <huff/Huffman52.hpp>
#include <huff/Knuth85.hpp>
#include <huff/Forward.hpp>
#include <huff/Hybrid.hpp>

using mtf_coder_t = MTFCoder<ASCIICoder, ASCIICoder, BinaryCoder<>>;

template<typename coder_t>
void test(const std::string& input, const std::string& filename, bool verify) {
    std::cout << "# " << filename << " ..." << std::endl;
    
    // encode using MTF first
    size_t bits_written;
    {
        std::string mtf_code;
        {
            std::ostringstream mtf_ss;
            {
                BitOStream out(mtf_ss);
                std::cout << "# encoding with MTF ..." << std::endl;
                
                encode<mtf_coder_t>(input, out);
                bits_written = out.bits_written();
            }
            mtf_code = mtf_ss.str();
        }
        
        // encode using given coder
        {
            std::ofstream f(filename);
            BitOStream out(f);

            std::cout << "# encoding file ..." << std::endl;
            encode<coder_t>(mtf_code, out);
            bits_written = out.bits_written();
        }
    }

    bool success;
    if(verify) {
        // decode using given coder
        {
            std::string mtf_dec;
            {
                std::ifstream f(filename);
                BitIStream in(f);

                std::cout << "# decoding file ..." << std::endl;
                mtf_dec = decode<coder_t>(in);
            }

            // decode MTF
            std::string dec;
            {
                std::istringstream ss(mtf_dec);
                BitIStream in(ss);

                std::cout << "# decoding with MTF ..." << std::endl;
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
        << ", rate=" << double(bits_written) / double(8 * input.length())
        << ", success=" << success
        << std::endl;
}

int main(int argc, char** argv) {
    tlx::CmdlineParser cp;

    std::string input_filename;
    cp.add_param_string("file", input_filename, "The input file.");

    bool verify = false;
    cp.add_flag("verify", verify, "Decode and verify.");

    if (!cp.process(argc, argv)) {
        return -1;
    }
    
    std::string text;
    {
        std::ifstream f(input_filename);
        f.seekg(0, std::ios::end);   
        text.reserve(f.tellg());
        f.seekg(0, std::ios::beg);

        text.assign((std::istreambuf_iterator<char>(f)),
                     std::istreambuf_iterator<char>());
    }
    
    test<ASCIICoder>(text, "ascii", verify);
    test<huff::Huffman52Coder<ASCIICoder, DeltaCoder>>(text, "huff52", verify);
    test<huff::Knuth85Coder<ASCIICoder>>(text, "knuth85", verify);
    test<huff::ForwardCoder<ASCIICoder, DeltaCoder>>(text, "fwd", verify);
    test<huff::HybridCoder<ASCIICoder, DeltaCoder>>(text, "hybrid", verify);
}

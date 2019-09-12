#include <iostream>
#include <fstream>
#include <streambuf>
#include <sstream>

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

template<typename coder_t>
void test(const std::string& input, const std::string& filename) {
    std::cout << "# " << filename << " ..." << std::endl;
    size_t bits_written;
    {
        std::ofstream f(filename);
        BitOStream out(f);

        std::cout << "# encoding ..." << std::endl;
        encode<coder_t>(input, out);
        bits_written = out.bits_written();
    }
    bool success;
    {
        std::ifstream f(filename);
        BitIStream in(f);

        std::cout << "# decoding ..." << std::endl;
        auto dec = decode<coder_t>(in);

        std::cout << "# checking ..." << std::endl;
        success = (dec == input);
    }

    std::cout << "RESULT algo=" << filename
        << ", in=" << 8 * input.length()
        << ", out=" << bits_written
        << ", rate=" << double(bits_written) / double(8 * input.length())
        << ", success=" << success
        << std::endl;
}

int main(int argc, char** argv) {
    std::string text;
    if(argc >= 2) {
        std::ifstream f(argv[1]);
        f.seekg(0, std::ios::end);   
        text.reserve(f.tellg());
        f.seekg(0, std::ios::beg);

        text.assign((std::istreambuf_iterator<char>(f)),
                     std::istreambuf_iterator<char>());
    } else {
        //text = "bananas&ananas";
        text = "ss&bnnnn$aaaaaa";
    }
    
    test<ASCIICoder>(text, "ascii");
    test<huff::Huffman52Coder<ASCIICoder, DeltaCoder>>(text, "huff52");
    test<huff::Knuth85Coder<ASCIICoder>>(text, "knuth85");
    test<huff::ForwardCoder<ASCIICoder, DeltaCoder>>(text, "fwd");
    test<huff::HybridCoder<ASCIICoder, DeltaCoder>>(text, "hybrid");
    test<MTFCoder<Delta0Coder, ASCIICoder, DeltaCoder>>(text, "mtf_delta");
}

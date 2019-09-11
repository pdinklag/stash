#include <iostream>
#include <fstream>
#include <sstream>

#include <code/ASCII.hpp>
#include <code/Binary.hpp>
#include <code/Delta.hpp>
#include <code/Delta0.hpp>
#include <code/MTF.hpp>

#include <io/BitOStream.hpp>
#include <io/BitIStream.hpp>

#include <huff/Huffman52.hpp>
#include <huff/Knuth85.hpp>
#include <huff/Forward.hpp>
#include <huff/Hybrid.hpp>

template<typename coder_t>
void test(const std::string& input, const std::string& filename) {
    std::cout << filename << ": ";
    {
        std::ofstream f(filename);
        BitOStream out(f);

        coder_t coder(input, out);
        for(size_t i = 0; i < input.size(); i++) {
            const uint8_t c = input[i];
            
            coder.encode(out, c);
            
            if(i+1 < input.size()) {
                coder.update(c);
            }
        }

        std::cout << out.bits_written() << " bits, decode ";
    }
    {
        std::ifstream f(filename);
        BitIStream in(f);
        coder_t decoder(in);

        std::ostringstream ss;
        while(!decoder.eof(in)) {
            const uint8_t c = decoder.decode(in);
            ss << c;
            decoder.update(c);
        }

        std::string result = ss.str();
        if(ss.str() == input) {
            std::cout << "SUCCESS";
        } else {
            std::cout << "FAIL: " << result;
        }
        std::cout << std::endl;
    }
}

int main(int argc, char** argv) {
    //std::string text("bananas&ananas");
    std::string text("ss&bnnnn$aaaaaa");
    test<ASCIICoder>(text, "ascii");
    test<huff::Huffman52Coder<ASCIICoder, DeltaCoder>>(text, "huff52_ascii");
    test<huff::Huffman52Coder<DeltaCoder, DeltaCoder>>(text, "huff52_delta");
    test<huff::Knuth85Coder<ASCIICoder>>(text, "knuth85_ascii");
    test<huff::Knuth85Coder<DeltaCoder>>(text, "knuth85_delta");
    test<huff::ForwardCoder<ASCIICoder, DeltaCoder>>(text, "fwd_ascii");
    test<huff::ForwardCoder<DeltaCoder, DeltaCoder>>(text, "fwd_delta");
    test<huff::HybridCoder<ASCIICoder, DeltaCoder>>(text, "hybrid_ascii");
    test<huff::HybridCoder<DeltaCoder, DeltaCoder>>(text, "hybrid_delta");
    test<MTFCoder<Delta0Coder, ASCIICoder, DeltaCoder>>(text, "mtf_delta");
}

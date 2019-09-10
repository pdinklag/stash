#include <iostream>
#include <fstream>
#include <sstream>

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

        if(ss.str() == input) {
            std::cout << "SUCCESS";
        } else {
            std::cout << "FAIL";
        }
        std::cout << std::endl;
    }
}

int main(int argc, char** argv) {
    //std::string text("bananas&ananas");
    std::string text("ss&bnnnn$aaaaaa");
    
    test<huff::Huffman52Coder>(text, "huff52");
    test<huff::Knuth85Coder>(text, "knuth85");
    test<huff::ForwardCoder>(text, "fwd");
    test<huff::HybridCoder>(text, "hybrid");
}

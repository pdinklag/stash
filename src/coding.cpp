#include <iostream>
#include <fstream>

#include <io/bitostream.hpp>
#include <io/bitistream.hpp>

#include <huff/Huffman52.hpp>
#include <huff/Knuth85.hpp>

template<typename coder_t>
void test(const std::string& input, const std::string& filename) {
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
    }
    {
        std::ifstream f(filename);
        BitIStream in(f);
        coder_t decoder(in);

        while(!in.eof()) {
            const uint8_t c = decoder.decode(in);
            std::cout << c;
            decoder.update(c);
        }
        std::cout << std::endl;
    }
}

int main(int argc, char** argv) {
    test<huff::Huffman52Coder>("bananas&ananas", "huff52");
    test<huff::Knuth85Coder>("bananas&ananas", "knuth85");
}

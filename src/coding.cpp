#include <iostream>
#include <fstream>

#include <io/bitostream.hpp>
#include <io/bitistream.hpp>

#include <huff/huffman52.hpp>
#include <huff/knuth85.hpp>

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
    test<huff::Huffman52Coder>("bananas&ananas", "file");
    test<huff::Knuth85Coder>("bananas&ananas", "file");

    /*
    std::ofstream f("file");
    BitOStream out(f);
    for(size_t i = 0; i < input.size(); i++) {
        const uint8_t c = input[i];

        coder.encode(out, c);
        
        if(i+1 < input.size()) {
            coder.increment(c);
        }
    }
    */
    
    /*
    {
        std::ofstream f("file");
        BitOStream out(f);
        huff::Knuth85Coder coder;

        auto input = std::string("bananas&ananas");
        for(size_t i = 0; i < input.size(); i++) {
            const uint8_t c = input[i];

            coder.encode(out, c);
            
            if(i+1 < input.size()) {
                coder.increment(c);
            }
        }
    }
    {
        std::ifstream f("file");
        BitIStream in(f);
        huff::Knuth85Coder decoder;

        while(!in.eof()) {
            const uint8_t c = decoder.decode(in);
            std::cout << c;
            decoder.increment(c);
        }
        std::cout << std::endl;
    }
    */
}

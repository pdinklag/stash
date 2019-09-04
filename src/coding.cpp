#include <iostream>
#include <fstream>

#include <io/bitostream.hpp>
#include <io/bitistream.hpp>

#include <huff/knuth85.hpp>

int main(int argc, char** argv) {
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

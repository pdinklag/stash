#pragma once

#include <vector>
#include <stash/huff/huffman_coder_base.hpp>

namespace stash {
namespace huff {

// Huffman coder based on [Huffman, 1952]
template<typename sym_coder_t, typename freq_coder_t>
class huffman_coder : public huffman_coder_base {
private:
    sym_coder_t  m_sym_coder;
    freq_coder_t m_freq_coder;

    inline huffman_coder() : huffman_coder_base() {
        // initialize
        for(size_t c = 0; c < MAX_SYMS; c++) {
            m_leaves[c] = nullptr;
        }
        m_num_nodes = 0;
    }

    // assumes that queue contains all the leaves!
    inline void build_tree(prio_queue_t& queue) {
        assert(!queue.empty());
        
        const size_t sigma = queue.size();
        for(size_t i = 0; i < sigma - 1; i++) {
            // get the next two nodes from the priority queue
            node_t* l = queue.top(); queue.pop();
            node_t* r = queue.top(); queue.pop();

            // make sure l has the smaller weight
            if(r->weight < l->weight) {
                std::swap(l, r);
            }

            // create a new node as parent of l and r
            node_t* v = node(m_num_nodes++);
            *v = node_t { l->weight + r->weight, 0, nullptr, 0, l, r, 0 };
            
            l->parent = v;
            l->bit = 0;
            r->parent = v;
            r->bit = 1;

            queue.push(v);
        }
        
        m_root = queue.top(); queue.pop();
        assert(queue.empty());
    }

public:
    inline huffman_coder(const std::string& s, bit_ostream& out) : huffman_coder() {
        // build Huffman tree and write histogram
        auto queue = init_leaves(s);
        build_tree(queue);
        encode_histogram(out, m_sym_coder, m_freq_coder);
    }

    inline huffman_coder(bit_istream& in) : huffman_coder() {
        // read histogram and build Huffman tree
        auto queue = decode_histogram(in, m_sym_coder, m_freq_coder);
        build_tree(queue);
    }

    inline void encode(bit_ostream& out, uint8_t c) {
        huffman_coder_base::encode(out, m_leaves[c]);
    }
};

}}

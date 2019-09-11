#pragma once

#include <vector>
#include <huff/HuffmanBase.hpp>

namespace huff {

// Huffman coder based on [Huffman, 1952]
template<typename sym_coder_t, typename freq_coder_t>
class Huffman52Coder : public HuffmanBase {
protected:
    sym_coder_t  m_sym_coder;
    freq_coder_t m_freq_coder;

    inline Huffman52Coder() : HuffmanBase() {
        // initialize
        for(size_t c = 0; c < MAX_SYMS; c++) {
            m_leaves[c] = nullptr;
        }
        m_num_nodes = 0;
    }

public:
    inline Huffman52Coder(const std::string& s, BitOStream& out) : Huffman52Coder() {
        // initialize Huffman tree leaves
        for(uint8_t c : s) {
            if(m_leaves[c]) {
                // already seen, increase weight
                ++m_leaves[c]->weight;
            } else {
                // first time this symbol occurs, create leaf
                node_t* q = node(m_num_nodes++);
                *q = node_t { 1, 0, nullptr, 0, nullptr, nullptr, c };
                m_leaves[c] = q;
            }
        }

        // enter all used symbols into a priority queue and write histogram to file
        prio_queue_t queue(node_priority_compare_t{});

        out.write_delta(m_num_nodes);
        for(size_t c = 0; c < MAX_SYMS; c++) {
            node_t* q = m_leaves[c];
            if(q) {
                assert(q->weight > 0);
                queue.push(q);
                
                m_sym_coder.encode(out, c);
                m_freq_coder.encode(out, q->weight);
            }
        }

        // build Huffman tree
        build_tree(queue);
    }

    inline Huffman52Coder(BitIStream& in) : Huffman52Coder() {
        //decode histogram, create leaves and fill queue
        prio_queue_t queue(node_priority_compare_t{});

        const size_t sigma = in.read_delta<>();
        for(size_t i = 0; i < sigma; i++) {
            const uint8_t c = m_sym_coder.template decode<uint8_t>(in);
            const size_t w = m_freq_coder.template decode<>(in);

            node_t* q = node(m_num_nodes++);
            *q = node_t { w, 0, nullptr, 0, nullptr, nullptr, c };
            m_leaves[c] = q;

            queue.push(q);
        }

        // build Huffman tree
        build_tree(queue);
    }

    inline void encode(BitOStream& out, uint8_t c) {
        const node_t* q = m_leaves[c];
        assert(q != nullptr);

        // encode
        std::vector<bool> bits;
        for(const node_t* v = q; v != m_root; v = v->parent) {
            bits.push_back(v->bit);
            
        }

        const node_t* x = m_root;
        for(size_t i = bits.size(); i > 0; i--) {
            out.write_bit(bits[i-1]);
            x = bits[i-1] ? x->right : x->left;
        }
    }

    inline uint8_t decode(BitIStream& in) {
        node_t* v = m_root;
        while(!v->leaf()) {
            v = in.read_bit() ? v->right : v->left;
        }
        return v->sym;
    }

    inline void update(uint8_t c) {
    }
};

}

#pragma once

#include <vector>
#include <huff/HuffmanBase.hpp>

namespace huff {

// Huffman coder based on [Huffman, 1952]
class Huffman52Coder : public HuffmanBase {
private:
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
                
                out.write_binary(uint8_t(c));
                out.write_delta(q->weight);
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
            const uint8_t c = in.template read_binary<uint8_t>();
            const size_t w = in.read_delta();

            node_t* q = node(m_num_nodes++);
            *q = node_t { w, 0, nullptr, 0, nullptr, nullptr, c };
            m_leaves[c] = q;

            queue.push(q);
        }

        // build Huffman tree
        build_tree(queue);
    }

    inline void encode(BitOStream& out, uint8_t c) const {
        const node_t* q = m_leaves[c];
        assert(q != nullptr);

        // encode
        std::vector<bool> bits;
        for(const node_t* v = q; v != m_root; v = v->parent) {
            bits.push_back(v->bit);
        }

        for(size_t i = bits.size(); i > 0; i--) {
            out.write_bit(bits[i-1]);
        }
    }

    inline uint8_t decode(BitIStream& in) const {
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

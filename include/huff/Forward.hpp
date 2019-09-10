#pragma once

#include <utility>
#include <vector>

#include <huff/Huffman52.hpp>
#include <io/BitIStream.hpp>
#include <io/BitOStream.hpp>

namespace huff {

// forward dynamic Huffman coding according to [Klein et al., 2019]
class ForwardCoder : public Huffman52Coder {
private:
    inline ForwardCoder() : Huffman52Coder() {
    }

public:
    inline ForwardCoder(const std::string& s, BitOStream& out) : Huffman52Coder(s, out) {
        // same as Huffman52
    }

    inline ForwardCoder(BitIStream& in) : Huffman52Coder(in) {
        // same as Huffman52
    }

    inline bool eof(BitIStream& in) const {
        if(in.eof()) {
            return !m_root;
        } else {
            return false;
        }
    }

    inline void encode(BitOStream& out, uint8_t c) const {
        if(m_root->leaf()) {
            // only root is left, and it's unique, no need to encode
        } else {
            // same as Huffman52
            Huffman52Coder::encode(out, c);
        }
    }

    inline uint8_t decode(BitIStream& in) const {
        if(m_root->leaf()) {
            // only root is left, report its symbol
            return m_root->sym;
        } else {
            // same as Huffman52
            return Huffman52Coder::decode(in);
        }
    }

    inline void update(uint8_t c) {
        assert(m_leaves[c]);
        
        for(node_t* p = m_leaves[c]; p != m_root; p = p->parent) {
            // find lowest ranked node with same weight as p
            node_t* q = p;
            const size_t w = p->weight;
            
            for(size_t i = 0; i < m_num_nodes; i++) {
                node_t* x = node(i);
                if(x->rank < p->rank && x->weight == w) {
                    q = x;
                }
            }

            if(p != q) {
                // interchange p and q
                node_t temp_q = *q;
                replace(q, p);
                replace(p, &temp_q);
            }

            --p->weight;
        }

        node_t* p = m_leaves[c];
        if(p == m_root) {
            // decrease weight, didn't happen above!
            assert(p->weight);
            --p->weight;
        }
        
        if(p->weight == 0) {
            // last occurrence of c
            if(p == m_root) {
                // last symbol
                m_leaves[c] = nullptr;
                m_root = nullptr;
            } else {
                // must have a parent, otherwise it's the root already
                node_t* parent = p->parent;
                assert(parent);

                // "delete" p and its parent by setting infinite weights
                p->weight = SIZE_MAX;
                parent->weight = SIZE_MAX;
                m_leaves[c] = nullptr;

                node_t* sibling = p->bit ? parent->left : parent->right;

                sibling->parent = parent->parent;
                sibling->bit = parent->bit;
                if(sibling->parent) {
                    if(sibling->bit) {
                        sibling->parent->right = sibling;
                    } else {
                        sibling->parent->left = sibling;
                    }
                } else {
                    assert(m_root == parent);
                    m_root = sibling;
                }
            }
        }
    }
};

}

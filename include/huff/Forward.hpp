#pragma once

#include <utility>
#include <vector>

#include <huff/Huffman52.hpp>

namespace huff {

// forward dynamic Huffman coding according to [Klein et al., 2019]
template<typename sym_coder_t, typename freq_coder_t>
class ForwardCoder : public Huffman52Coder<sym_coder_t, freq_coder_t> {
private:
    using Base = Huffman52Coder<sym_coder_t, freq_coder_t>;
    using node_t = typename Base::node_t;

    using Base::node;
    using Base::replace;
    using Base::m_root;
    using Base::m_leaves;
    using Base::m_num_nodes;

    inline ForwardCoder() : Base() {
    }

public:
    inline ForwardCoder(const std::string& s, BitOStream& out) : Base(s, out) {
        // same as Huffman52
    }

    inline ForwardCoder(BitIStream& in) : Base(in) {
        // same as Huffman52
    }

    inline bool eof(BitIStream& in) const {
        if(in.eof()) {
            return !m_root;
        } else {
            return false;
        }
    }

    inline void encode(BitOStream& out, uint8_t c) {
        if(m_root->leaf()) {
            // only root is left, and it's unique, no need to encode
        } else {
            // same as Huffman52
            Base::encode(out, c);
        }

        // decrease weight
        decrease(c);
    }

    inline uint8_t decode(BitIStream& in) {
        uint8_t c;
        if(m_root->leaf()) {
            // only root is left, report its symbol
            c = m_root->sym;
        } else {
            // same as Huffman52
            c = Base::decode(in);
        }

        // decrease weight
        decrease(c);
        return c;
    }

private:
    inline void decrease(uint8_t c) {
        assert(m_leaves[c]);
        
        for(node_t* p = m_leaves[c]; p; p = p->parent) {
            // find lowest ranked node q with same weight as p
            node_t* q = p;
            const size_t w = p->weight;
            
            for(size_t i = 0; i < m_num_nodes; i++) {
                node_t* x = node(i);
                if(x->rank < q->rank && x->weight == w) {
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

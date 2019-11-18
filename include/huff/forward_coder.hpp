#pragma once

#include <utility>
#include <vector>

#include <huff/huffman_coder_base.hpp>

namespace huff {

// forward dynamic Huffman coding according to [Klein et al., 2019]
template<typename sym_coder_t, typename freq_coder_t>
class forward_coder : public adaptive_huffman_coder_base {
private:
    sym_coder_t  m_sym_coder;
    freq_coder_t m_freq_coder;

    inline forward_coder() : adaptive_huffman_coder_base() {
        // initialize
        for(size_t c = 0; c < MAX_SYMS; c++) {
            m_leaves[c] = nullptr;
        }
        m_num_nodes = 0;
    }

public:
    inline forward_coder(const std::string& s, bit_ostream& out) : forward_coder() {
        // build Huffman tree and write histogram
        auto queue = init_leaves(s);
        build_tree(queue);
        encode_histogram(out, m_sym_coder, m_freq_coder);
    }

    inline forward_coder(bit_istream& in) : forward_coder() {
        // read histogram and build Huffman tree
        auto queue = decode_histogram(in, m_sym_coder, m_freq_coder);
        build_tree(queue);
    }

    inline bool eof(bit_istream& in) const {
        if(in.eof()) {
            return !m_root;
        } else {
            return false;
        }
    }

    inline void encode(bit_ostream& out, uint8_t c) {
        if(m_root->leaf()) {
            // only root is left, and it's unique, no need to encode
        } else {
            // default
            huffman_coder_base::encode(out, m_leaves[c]);
        }

        // decrease weight
        decrease(c);
    }

    inline uint8_t decode(bit_istream& in) {
        uint8_t c;
        if(m_root->leaf()) {
            // only root is left, report its symbol
            c = m_root->sym;
        } else {
            // default
            c = huffman_coder_base::decode(in);
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

            for(size_t rank = p->rank; rank > 0; rank--) {
                node_t* x = m_rank_map[rank-1];
                if(x) {
                    if(x->weight == w) {
                        q = x;
                    } else if(x->weight < w) {
                        break;
                    }
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
                m_rank_map[p->rank] = nullptr;
                parent->weight = SIZE_MAX;
                m_rank_map[parent->rank] = nullptr;
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

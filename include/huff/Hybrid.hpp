#pragma once

#include <utility>
#include <vector>

#include <huff/AdaptiveHuffmanBase.hpp>

namespace huff {

// forward dynamic Huffman coding according to [Fruchtman et al., 2019]
template<typename sym_coder_t, typename freq_coder_t>
class HybridCoder : public AdaptiveHuffmanBase {
private:
    sym_coder_t  m_sym_coder;
    freq_coder_t m_freq_coder;

    size_t m_hist[MAX_SYMS];

    inline HybridCoder() : AdaptiveHuffmanBase() {
        // initialize
        for(size_t c = 0; c < MAX_SYMS; c++) {
            m_leaves[c] = nullptr;
            m_hist[c] = 0;
        }
        m_num_nodes = 0;
    }

public:
    inline HybridCoder(const std::string& s, BitOStream& out) : HybridCoder() {
        // count histogram
        size_t sigma = 0;
        for(uint8_t c : s) {
            if(!m_hist[c]) ++sigma;
            ++m_hist[c];
        }

        // init tree with NYT node
        m_num_nodes = 1;
        m_nodes[0] = node_t{ sigma, 0, nullptr, 0, nullptr, nullptr, 0 };
        m_root = node(0);

        // write sigma
        m_freq_coder.encode(out, sigma);
    }

    inline HybridCoder(BitIStream& in) : HybridCoder() {
        // read sigma
        const size_t sigma = m_freq_coder.template decode<>(in);

        // init tree with NYT node
        m_num_nodes = 1;
        m_nodes[0] = node_t{ sigma, 0, nullptr, 0, nullptr, nullptr, 0 };
        m_root = node(0);
    }

    inline bool eof(BitIStream& in) const {
        if(in.eof()) {
            return !m_root;
        } else {
            return false;
        }
    }

    inline void encode(BitOStream& out, uint8_t c) {
        const node_t* q = m_leaves[c];
        if(q == m_root) {
            // only symbol in existance, no need to encode
        } else {
            // mostly like Knuth85
            bool is_nyt;
            if(q) {
                is_nyt = false;
            } else {
                q = node(0);
                is_nyt = true;
            }

            // encode symbol unless it is the first
            if(!(is_nyt && m_num_nodes == 1)) {
                std::vector<bool> bits;
                for(const node_t* v = q; v != m_root; v = v->parent) {
                    bits.push_back(v->bit);
                }

                for(size_t i = bits.size(); i > 0; i--) {
                    out.write_bit(bits[i-1]);
                }
            }

            // if symbol is NYT, encode ASCII encoding AND frequency
            if(is_nyt) {
                m_sym_coder.encode(out, c);
                m_freq_coder.encode(out, m_hist[c]);
            }
        }

        // update
        update(c);
    }

    inline uint8_t decode(BitIStream& in) {
        uint8_t c;
        if(in.eof()) {
            // last symbol
            assert(m_root && m_root->leaf());
            c = m_root->sym;
        } else {
            // nearly the same as Knuth85
            if(m_num_nodes == 1) {
                // first character
                c = m_sym_coder.template decode<uint8_t>(in);
                m_hist[c] = m_freq_coder.template decode<>(in);
            } else {
                node_t* v = m_root;
                while(!v->leaf()) {
                    v = in.read_bit() ? v->right : v->left;
                }

                if(v == node(0)) {
                    // NYT
                    c = m_sym_coder.template decode<uint8_t>(in);
                    m_hist[c] = m_freq_coder.template decode<>(in);
                } else {
                    c = v->sym;
                }
            }
        }

        // update
        update(c);
        return c;
    }

private:
    inline void decrease(node_t* leaf) {
        assert(leaf->leaf());

        // practically the same as Forward::update
        for(node_t* p = leaf; p; p = p->parent) {
            // find lowest ranked node with same weight as p
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
                assert(q != m_root);

                // interchange p and q
                node_t temp_q = *q;
                replace(q, p);
                replace(p, &temp_q);
            }

            assert(p->weight > 0);
            --p->weight;
        }

        node_t* p = leaf;
        if(p->weight == 0) {
            // last occurrence of the character
            if(p == m_root) {
                // last symbol
                m_leaves[m_root->sym] = nullptr;
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

                if(p != node(0)) {
                    m_leaves[p->sym] = nullptr;
                }

                node_t* sibling = p->bit ? parent->left : parent->right;
                assert(sibling);

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
                    assert(!sibling->parent);
                    m_root = sibling;
                }
            }
        }
    }

    inline void insert(uint8_t c) {
        const size_t w = m_hist[c] - 1;

        // only insert if it's going to occur again
        if(w > 0) {
            // find the highest ranked node v with weight <= w
            // or, if it does not exist, a node with minimum weight
            node_t* v = nullptr;
            node_t* min = nullptr;
            for(size_t rank = 0; rank < MAX_SYMS; rank++) {
                node_t* x = m_rank_map[rank];
                if(x) {
                    if(!min) min = x;
                    if(x->weight > w) {
                        break;
                    } else {
                        v = x;
                    }
                }
            }

            for(size_t i = 0; i < m_num_nodes; i++) {
                node_t* x = node(i);
                if((!v || x->rank > v->rank) && x->weight <= w) {
                    v = x;
                }

                if(!min || x->weight < min->weight) min = x;
            }

            if(!v) v = min;
            assert(v); // there must be at least NYT

            // increase ranks of nodes with rank > v->rank by two
            // (we will insert nodes with ranks v->rank+1 and v->rank+2)
            for(size_t i = 0; i < m_num_nodes; i++) {
                node_t* x = node(i);
                if(x->rank > v->rank) set_rank(x, x->rank + 2, false);
                assert(x->rank < MAX_NODES);
            }

            // create a new leaf q for c
            node_t* q = node(m_num_nodes++);
            m_leaves[c] = q;

            // create a new inner node u at the place of v
            // with left child v (so ranks remain) and right child the new leaf
            node_t* u = node(m_num_nodes++);
            *u = node_t { v->weight+w, v->rank+2, v->parent, v->bit, v, q, 0 };

            // properly assign rank
            assert(u->rank < MAX_NODES);
            assert(m_rank_map[u->rank] == nullptr);
            set_rank(u, u->rank, true);

            if(v->parent) {
                if(v->bit) {
                    v->parent->right = u;
                } else {
                    v->parent->left = u;
                }
            }
            v->parent = u;
            v->bit = 0;

            if(v == m_root) {
                assert(!u->parent);
                m_root = u;
            }

            // init leaf
            *q = node_t { w, v->rank+1, u, 1, nullptr, nullptr, c };

            // properly assign rank
            assert(q->rank < MAX_NODES);
            assert(m_rank_map[q->rank] == nullptr);
            set_rank(q, q->rank, true);

            // increase weights up
            for(v = u->parent; v; v = v->parent) {
                v->weight += w;
            }
        }
    }

    inline void update(uint8_t c) {
        if(m_leaves[c]) {
            decrease(m_leaves[c]);
        } else {
            insert(c);
            decrease(node(0)); // consume a NYT
        }
    }
};

}

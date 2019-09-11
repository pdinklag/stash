#pragma once

#include <utility>
#include <vector>

#include <huff/HuffmanBase.hpp>
#include <io/BitIStream.hpp>
#include <io/BitOStream.hpp>

namespace huff {

// dynamic (online) Huffman coding according to [Knuth, 1985]
template<typename sym_coder_t>
class Knuth85Coder : public HuffmanBase {
private:
    sym_coder_t m_sym_coder;

    inline Knuth85Coder() : HuffmanBase() {
        // init tree with NYT node
        m_num_nodes = 1;
        m_nodes[0] = node_t{ 0, 0, nullptr, 0, nullptr, nullptr, 0 };
        m_root = node(0);

        // initialize symbol index
        for(size_t c = 0; c < MAX_SYMS; c++) {
            m_leaves[c] = nullptr;
        }
    }

public:
    inline Knuth85Coder(const std::string& s, BitOStream& out) : Knuth85Coder() {
    }

    inline Knuth85Coder(BitIStream& in) : Knuth85Coder() {
    }

    inline void encode(BitOStream& out, uint8_t c) {
        const node_t* q = m_leaves[c];
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

        // if symbol is NYT, encode ASCII encoding
        if(is_nyt) {
            m_sym_coder.encode(out, c);
        }

        // increase weight
        increase(c);
    }

    inline uint8_t decode(BitIStream& in) {
        uint8_t c;
        if(m_num_nodes == 1) {
            // first character
            c = m_sym_coder.template decode<uint8_t>(in);
        } else {        
            node_t* v = m_root;
            while(!v->leaf()) {
                v = in.read_bit() ? v->right : v->left;
            }

            if(v == node(0)) {
                // NYT
                c = m_sym_coder.template decode<uint8_t>(in);
            } else {
                c = v->sym;
            }
        }

        // increase weight
        increase(c);
        return c;
    }

private:
    inline void increase(uint8_t c) {
        // find or create leaf for c
        node_t* nyt = node(0);
        
        node_t* q = m_leaves[c];
        if(!q) {
            // new symbol - we need to add a leaf for it in the tree

            // since we will add two nodes (leaf and inner node)
            // with zero weight, they will be rank 1 and 2, respectively
            // therefore, increase the ranks of all current nodes by two
            // (except NYT, which remains 0)
            for(size_t i = 1; i < m_num_nodes; i++) {
                m_nodes[i].rank += 2;
            }
            
            // create new inner node v
            node_t* v = node(m_num_nodes++);

            // create new leaf q
            q = node(m_num_nodes++);
            m_leaves[c] = q;

            // initialize v by replacing NYT
            *v = node_t { 0, 2, nyt->parent, nyt->bit, nyt, q, 0 };
            if(nyt->parent) nyt->parent->left = v;

            // make NYT left child of v
            nyt->parent = v;
            nyt->bit = 0; // TODO: is this necessary?

            // if root is still NYT, v becomes the root and stays root forever
            if(m_root == nyt) m_root = v;

            // initialize q as right child of v
            *q = node_t { 0, 1, v, 1, nullptr, nullptr, c };
        }

        // if q is the sibling of NYT, we need to move it elsewhere
        // by the structure of ranks, q is the sibling of NYT
        // if and only if its rank is 1
        if(q->rank == 1) {
            // identify the leaf of highest rank  with the same weight as q
            // denote it as u
            const size_t w = q->weight;
            node_t* u = q;
            for(size_t i = 1; i < m_num_nodes; i++) {
                node_t* x = node(i);
                if(x->leaf() && x->rank > u->rank && x->weight == w) {
                    u = x;
                }
            }

            if(u != q) {
                // interchange q and u
                node_t temp_u = *u;
                replace(u, q);
                replace(q, &temp_u);
            }

            ++q->weight;
            q = q->parent;
        }

        // we now move up the tree, starting with current node v = q
        // and restructure the tree so incrementing the frequency becomes easy
        // [Vitter 1985] calls this the "first phase"
        for(node_t* v = q; v; v = v->parent) {
            // first, we identify the node of highest rank with the same weight
            // we denote that node as u
            // NYT can always be skipped
            const size_t w = v->weight;
            node_t* u = v;
            for(size_t i = 1; i < m_num_nodes; i++) {
                node_t* x = node(i);
                if(x->rank > u->rank && x->weight == w) {
                    u = x;
                }
            }

            if(u != v) {
                // interchange u and v
                node_t temp_u = *u;
                replace(u, v);
                replace(v, &temp_u);
            }
        }

        // now we simply increment the weights up the tree from q
        for(; q; q = q->parent) ++q->weight;
    }
};

}

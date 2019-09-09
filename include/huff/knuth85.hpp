#pragma once

#include <utility>
#include <vector>

#include <io/bitostream.hpp>

#define DEBUG_KNUTH_85

#include <cassert>
#include <iostream>

namespace huff {

// dynamic (online) Huffman coding according to [Knuth, 1985]
class Knuth85Coder {
private:
    static constexpr size_t MAX_SYMS = 256ULL;
    static constexpr size_t MAX_NODES = 512ULL;

private:
    struct node_t {
        size_t weight;
        size_t rank;
        
        node_t* parent;
        bool bit;

        node_t* left;
        node_t* right;

        inline bool leaf() {
            return !(left || right);
        }
    };

    node_t m_nodes[MAX_NODES];
    size_t m_num_nodes;
    
    node_t* m_leaves[MAX_SYMS];
    node_t* m_root;

    inline node_t* node(size_t v) {
        return &m_nodes[v];
    }

    inline void replace(node_t* u, node_t* v) {
        u->parent = v->parent;
        u->rank = v->rank;
        u->bit = v->bit;

        if(u->parent) {
            if(u->bit) {
                u->parent->right = u;
            } else {
                u->parent->left = u;
            }
        }
    }

public:
    inline Knuth85Coder() {
        // init tree with NYT node
        m_num_nodes = 1;
        m_nodes[0] = node_t{ 0, 0, nullptr, 0, nullptr, nullptr };
        m_root = node(0);

        // initialize symbol index
        for(size_t c = 0; c < MAX_SYMS; c++) {
            m_leaves[c] = nullptr;
        }
    }

    inline void encode(BitOStream& out, uint8_t c) {
        #ifdef DEBUG_KNUTH_85
        std::cout << "encode: '" << c << "'";
        #endif
        
        node_t* q = m_leaves[c];
        bool is_nyt;
        if(q) {
            is_nyt = false;
        } else {
            q = node(0);
            is_nyt = true;

            #ifdef DEBUG_KNUTH_85
            std::cout << " (NYT)";
            #endif
        }

        #ifdef DEBUG_KNUTH_85
        std::cout << std::endl << "->";
        #endif

        // encode symbol unless it is the first
        if(!(is_nyt && m_num_nodes == 1)) {
            std::vector<bool> bits;
            for(node_t* v = q; v != m_root; v = v->parent) {
                bits.push_back(v->bit);
            }

            #ifdef DEBUG_KNUTH_85
            node_t* x = m_root;
            std::cout << " ";
            #endif
            for(size_t i = bits.size(); i > 0; i--) {
                out.write_bit(bits[i-1]);
                
                #ifdef DEBUG_KNUTH_85
                x = bits[i-1] ? x->right : x->left;
                std::cout << size_t(bits[i-1]);
                #endif
            }

            #ifdef DEBUG_KNUTH_85
            assert(x == q);
            #endif
        }

        // if symbol is NYT, encode ASCII encoding
        if(is_nyt) {
            out.write_binary(c);
            #ifdef DEBUG_KNUTH_85
            std::cout << " ASCII('" << c << "')";
            #endif
        }

        #ifdef DEBUG_KNUTH_85
        std::cout << std::endl;
        #endif
    }

    inline void increment(uint8_t c) {
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
            *v = node_t { 0, 2, nyt->parent, nyt->bit, nyt, q };
            if(nyt->parent) nyt->parent->left = v;

            // make NYT left child of v
            nyt->parent = v;
            nyt->bit = 0; // TODO: is this necessary?

            // if root is still NYT, v becomes the root and stays root forever
            if(m_root == nyt) m_root = v;

            // initialize q as right child of v
            *q = node_t { 0, 1, v, 1, nullptr, nullptr };
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
                #ifdef DEBUG_KNUTH_85
                std::cout << "interchange (leaf): " << q->rank << " and " << u->rank << std::endl;
                #endif
                
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
                #ifdef DEBUG_KNUTH_85
                std::cout << "interchange (inner): " << v->rank << " and " << u->rank << std::endl;
                //std::cout << "\tBEFORE: u=" << *u << ", v=" << *v << std::endl;
                #endif
                
                node_t temp_u = *u;
                replace(u, v);
                replace(v, &temp_u);

                #ifdef DEBUG_KNUTH_85
                //std::cout << "\tAFTER: u=" << *u << ", v=" << *v << std::endl;
                #endif
            }
        }

        // now we simply increment the weights up the tree from q
        for(; q; q = q->parent) ++q->weight;
    }
};

}

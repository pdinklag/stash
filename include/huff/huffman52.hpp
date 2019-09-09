#pragma once

#include <cassert>
#include <algorithm>
#include <queue>
#include <utility>
#include <vector>

#include <io/bitistream.hpp>
#include <io/bitostream.hpp>

namespace huff {

// Huffman coder based on [Huffman, 1952]
class Huffman52Coder {
private:
    static constexpr size_t MAX_SYMS = 256ULL;
    static constexpr size_t MAX_NODES = 512ULL;

    struct node_t {
        size_t weight;
        size_t rank;
        
        node_t* parent;
        bool bit;

        node_t* left;
        node_t* right;

        uint8_t sym;

        inline bool leaf() {
            return !(left || right);
        }
    };

    struct node_priority_compare_t {
        inline bool operator()(node_t* a, node_t* b) {
            return a->weight >= b->weight;
        };
    };

    using prio_queue_t = std::priority_queue<
        node_t*, std::vector<node_t*>, node_priority_compare_t>;

    node_t m_nodes[MAX_NODES];
    size_t m_num_nodes;
    
    node_t* m_leaves[MAX_SYMS];
    node_t* m_root;

    inline node_t* node(size_t v) {
        return &m_nodes[v];
    }

    inline const node_t* node(size_t v) const {
        return &m_nodes[v];
    }

    inline void build_tree(prio_queue_t& queue) {
        size_t next_rank = 0;
        const size_t sigma = queue.size();
        for(size_t i = 0; i < sigma - 1; i++) {
            // get the next two nodes from the priority queue
            node_t* l = queue.top(); queue.pop();
            node_t* r = queue.top(); queue.pop();

            // make sure l has the smaller weight
            if(r->weight < l->weight) {
                std::swap(l, r);
            }

            // determine ranks
            l->rank = next_rank++;  
            r->rank = next_rank++;

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
        m_root->rank = next_rank++;
        assert(queue.empty());
    }

    inline Huffman52Coder() {
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

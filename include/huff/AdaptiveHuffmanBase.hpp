#pragma once

#include <huff/HuffmanBase.hpp>
#include <io/BitOStream.hpp>

namespace huff {

class AdaptiveHuffmanBase : public HuffmanBase {
protected:
    node_t* m_rank_map[MAX_NODES];

    inline void set_rank(node_t* v, size_t rank, bool initial) {
        assert(rank < MAX_NODES);

        if(!initial) m_rank_map[v->rank] = nullptr;
        v->rank = rank;
        m_rank_map[rank] = v;
    }

    // assumes that queue contains all the leaves!
    // same as Huffman52Coder::build_tree, except we also set the ranks here
    inline void build_tree(prio_queue_t& queue) {
        assert(!queue.empty());

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
            set_rank(l, next_rank++, true);
            set_rank(r, next_rank++, true);

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
        set_rank(m_root, next_rank++, true);
        assert(queue.empty());
    }

    // same as HuffmanBase, but using set_rank
    inline void replace(node_t* u, node_t* v) {
        u->parent = v->parent;
        set_rank(u, v->rank, false);
        u->bit = v->bit;

        if(u->parent) {
            if(u->bit) {
                u->parent->right = u;
            } else {
                u->parent->left = u;
            }
        }
    }

    using HuffmanBase::HuffmanBase;
};

}

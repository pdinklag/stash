#pragma once

#include <cassert>
#include <queue>
#include <utility>

namespace huff {

class HuffmanBase {
protected:
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

    // assumes that queue contains all the leaves!
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

    inline HuffmanBase() {
    }
};

}

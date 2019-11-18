#pragma once

#include <cassert>
#include <queue>
#include <utility>
#include <vector>

#include <code/coder.hpp>

namespace huff {

class huffman_coder_base : public coder {
protected:
    static constexpr size_t MAX_SYMS = 256ULL;
    static constexpr size_t MAX_NODES = 514ULL; // 2 * 257 (all bytes + NYT)

    struct node_t {
        size_t weight;
        size_t rank; // only used by adaptive algorithms

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

    inline huffman_coder_base() {
    }

    inline node_t* node(size_t v) {
        assert(v < MAX_NODES);
        return &m_nodes[v];
    }

    inline const node_t* node(size_t v) const {
        assert(v < MAX_NODES);
        return &m_nodes[v];
    }

    inline prio_queue_t init_leaves(const std::string& s) {
        // init leaves
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

        // insert leaves into priority queue
        prio_queue_t queue(node_priority_compare_t{});
        for(size_t c = 0; c < MAX_SYMS; c++) {
            node_t* q = m_leaves[c];
            if(q) {
                assert(q->weight > 0);
                queue.push(q);
            }
        }

        return queue;
    }

    template<typename sym_coder_t, typename freq_coder_t>
    inline void encode_histogram(
        bit_ostream& out,
        sym_coder_t& sym_coder,
        freq_coder_t& freq_coder) {

        size_t sigma = 0;
        for(size_t c = 0; c < MAX_SYMS; c++) {
            if(m_leaves[c]) ++sigma;
        }

        out.write_delta(sigma);
        for(size_t c = 0; c < MAX_SYMS; c++) {
            node_t* q = m_leaves[c];
            if(q) {
                sym_coder.encode(out, q->sym);
                freq_coder.encode(out, q->weight);
            }
        }
    }

    template<typename sym_coder_t, typename freq_coder_t>
    inline prio_queue_t decode_histogram(
        bit_istream& in,
        sym_coder_t& sym_coder,
        freq_coder_t& freq_coder) {

        //decode histogram, create leaves and fill queue
        prio_queue_t queue(node_priority_compare_t{});

        const size_t sigma = in.read_delta<>();
        for(size_t i = 0; i < sigma; i++) {
            const uint8_t c = sym_coder.template decode<uint8_t>(in);
            const size_t w = freq_coder.template decode<>(in);

            node_t* q = node(m_num_nodes++);
            *q = node_t { w, 0, nullptr, 0, nullptr, nullptr, c };
            m_leaves[c] = q;

            queue.push(q);
        }
        return queue;
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
    inline void encode(bit_ostream& out, node_t* leaf) {
        assert(leaf);

        // encode
        std::vector<bool> bits;
        for(const node_t* v = leaf; v != m_root; v = v->parent) {
            bits.push_back(v->bit);
        }

        for(size_t i = bits.size(); i > 0; i--) {
            out.write_bit(bits[i-1]);
        }
    }

    inline uint8_t decode(bit_istream& in) {
        node_t* v = m_root;
        while(!v->leaf()) {
            v = in.read_bit() ? v->right : v->left;
        }
        return v->sym;
    }
};

}

#pragma once

#include <huff/HuffmanBase.hpp>

namespace huff {
    
class DynamicHuffmanBase : public HuffmanBase
{
protected:
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

    inline DynamicHuffmanBase() : HuffmanBase() {
    }
};

}

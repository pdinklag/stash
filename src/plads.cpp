#include <bitset>
#include <cassert>
#include <iostream>
#include <vector>

#include <plads/graph/graph.hpp>

template<typename graph_t>
void test_graph() {
    static std::vector<bool> adj = {
                /* 0, 1, 2, 3, 4 */
        /* 0 */    0, 1, 0, 0, 0,
        /* 1 */    0, 0, 1, 1, 1,
        /* 2 */    1, 0, 0, 1, 0,
        /* 3 */    0, 0, 0, 0, 0,
        /* 4 */    0, 0, 0, 0, 0
    };
    
    graph_t g(adj);

    const auto n = g.number_of_nodes();
    assert(n == 5);
    assert(g.size() == n);
    assert(g.number_of_edges() == 6);
    assert(g.degree(0) == 1);
    assert(g.degree(1) == 3);
    assert(g.degree(2) == 2);
    assert(g.degree(3) == 0);
    assert(g.degree(4) == 0);

    {
        for(uint64_t u = 0; u < n; u++) {
            // make sure every directly reachable node v from u is also a neighbour of u
            for(uint64_t i = 0; i < g.degree(u); i++) {
                uint64_t v = g.get(u, i);
                assert(g.is_neighbour(u, v));
            }

            // make sure neighbour relation matches adjacency matrix
            for(uint64_t v = 0; v < n; v++) {
                assert(g.is_neighbour(u, v) == adj[u * n + v]);
            }
        }
    }
}

int main(int argc, char** argv) {
    test_graph<graph>();
    std::cout << "SUCCESS" << std::endl;
}

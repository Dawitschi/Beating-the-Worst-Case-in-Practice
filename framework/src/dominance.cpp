#include "framework/dominance.hpp"
#include "framework/graph.hpp"

#include <vector>
// Function to reduce a graph by removing dominated vertices 
Graph reduce_dominance(const Graph& G) {

    bool dominance_vertex = true;
    unsigned n = G.n();

    std::vector<char> mark(n, 0);
    std::vector<char> dominance_vertices(n, 0);

    //tracking the degree of each vertex
    std::vector<unsigned> deg(n, 0);
    for (node v = 0; v < n; ++v) {
        deg[v] = G.neighbors(v).size();
    }

    //Applying until no dominant vertices remain to reduce the graph
    while (dominance_vertex) {
        dominance_vertex = false;

        for (node b = 0; b < n; ++b) {
            if (dominance_vertices[b] == 1 || deg[b] == 0) continue;
            const std::vector<node>& neighbors_of_b = G.neighbors(b);
    
            //using array to mark neighbors of b
            mark[b] = 1;
            for (node neighbor : neighbors_of_b) {
                if (dominance_vertices[neighbor] == 0) {
                    mark[neighbor] = 1;
                }
            }

            bool b_dominates = false;

            for (node a : neighbors_of_b) {
                if (dominance_vertices[a] == 1) continue;

                if (deg[b] < deg[a]) continue;

                const std::vector<node>& neighbors_of_a = G.neighbors(a);
                bool a_is_subset = true;

                for (node c : neighbors_of_a) {
                    if (dominance_vertices[c] == 1) continue;

                    if (mark[c] == 0) {
                        a_is_subset = false;
                        break;
                    }
                }

                if (a_is_subset) {
                    b_dominates = true;
                    break;
                }
            }

            mark[b] = 0;
            for (node neighbor : neighbors_of_b) {
                mark[neighbor] = 0;
            }

            if (b_dominates) {
                dominance_vertices[b] = 1;
                dominance_vertex = true;
                for (node neighbor : neighbors_of_b) {
                    if (dominance_vertices[neighbor] == 0) {
                        deg[neighbor]--;
                    }
                }
            }

            
        }
    }

    // Collect remaining nodes that are not dominated and not isolated to form the reduced graph
    std::vector<node> remaining_nodes;
    for (node v = 0; v < n; ++v) {
        if (dominance_vertices[v] == 0 && deg[v] > 0) {
            remaining_nodes.push_back(v);
        }
    }

    return Graph(G, remaining_nodes);
}
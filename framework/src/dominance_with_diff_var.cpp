#include "framework/dominance_with_diff_var.hpp"
#include "framework/graph.hpp"
#include <vector>
#include <unordered_set>
#include <algorithm>

Graph reduce_dominance_hash(const Graph& G) {
    bool dominance_vertex = true;
    unsigned n = G.n();
    unsigned iterations = 0;

    std::vector<char> dominance_vertices(n, 0);
    std::vector<unsigned> deg(n, 0);

    for (node v = 0; v < n; ++v) {
        deg[v] = G.neighbors(v).size();
    }

    while (dominance_vertex) {
        dominance_vertex = false;
        iterations++;

        for (node b = 0; b < n; ++b) {
            if (dominance_vertices[b] == 1 || deg[b] == 0) continue;

            const std::vector<node>& neighbors_of_b = G.neighbors(b);
            
            std::unordered_set<node> hash_b;
            hash_b.reserve(neighbors_of_b.size());
            for (node neighbor : neighbors_of_b) {
                if (dominance_vertices[neighbor] == 0) {
                    hash_b.insert(neighbor);
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
                    
                    if (hash_b.find(c) == hash_b.end()) {
                        a_is_subset = false;
                        break;
                    }
                }

                if (a_is_subset) {
                    b_dominates = true;
                    break;
                }
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

    std::vector<node> remaining_nodes;
    for (node v = 0; v < n; ++v) {
        if (dominance_vertices[v] == 0 && deg[v] > 0) {
            remaining_nodes.push_back(v);
        }
    }

    return Graph(G, remaining_nodes);
}


Graph reduce_dominance_sorted(const Graph& G) {
    bool dominance_vertex = true;
    unsigned n = G.n();
    unsigned iterations = 0;

    std::vector<char> dominance_vertices(n, 0);
    std::vector<unsigned> deg(n, 0);
    std::vector<std::vector<node>> sorted_adj(n);

    for (node v = 0; v < n; ++v) {
        sorted_adj[v] = G.neighbors(v);
        std::sort(sorted_adj[v].begin(), sorted_adj[v].end());
        deg[v] = sorted_adj[v].size();
    }

    while (dominance_vertex) {
        dominance_vertex = false;
        iterations++;

        for (node b = 0; b < n; ++b) {
            if (dominance_vertices[b] == 1 || deg[b] == 0) continue;

            const std::vector<node>& neighbors_of_b = sorted_adj[b];
            bool b_dominates = false;

            for (node a : neighbors_of_b) {
                if (dominance_vertices[a] == 1) continue;
                if (deg[b] < deg[a]) continue;

                const std::vector<node>& neighbors_of_a = sorted_adj[a];
                bool a_is_subset = true;

                size_t idx_a = 0; 
                size_t idx_b = 0; 

                while (idx_a < neighbors_of_a.size() && idx_b < neighbors_of_b.size()) {
                    node neighbor_a = neighbors_of_a[idx_a];
                    node neighbor_b = neighbors_of_b[idx_b];

                    if (dominance_vertices[neighbor_a] == 1) { idx_a++; continue; }
                    if (dominance_vertices[neighbor_b] == 1) { idx_b++; continue; }

                    if (neighbor_a < neighbor_b) {
                        a_is_subset = false;
                        break;
                    } else if (neighbor_a > neighbor_b) {
                        idx_b++;
                    } else {
                        idx_a++;
                        idx_b++;
                    }
                }

                if (a_is_subset) {
                    while (idx_a < neighbors_of_a.size()) {
                        if (dominance_vertices[neighbors_of_a[idx_a]] == 0) {
                            a_is_subset = false;
                            break;
                        }
                        idx_a++;
                    }
                }

                if (a_is_subset) {
                    b_dominates = true;
                    break;
                }
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


    std::vector<node> remaining_nodes;
    for (node v = 0; v < n; ++v) {
        if (dominance_vertices[v] == 0 && deg[v] > 0) {
            remaining_nodes.push_back(v);
        }
    }

    return Graph(G, remaining_nodes);
}
#include "graph.hpp"

struct SearchResult {
    std::vector<node> path;
    unsigned path_length = 0;
    std::size_t expanded_nodes = 0;
    std::size_t examined_edges = 0;
};

SearchResult bfs(const Graph& G, node source, node target ); 
SearchResult bibfs(const Graph& G, node source, node target);

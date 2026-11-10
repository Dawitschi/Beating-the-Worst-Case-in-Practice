#include "framework/bbfs.hpp"
#include <set>
#include <map>
#include <vector>

// TODO: More efficient visited logic.

// Find a shortest path in the unweighted graph using a FIFO queue. Discover
// nodes in increasing distance from source and record each node's first parent.
// Stop when target is dequeued, then follow parents from target back to source;
// the returned path is therefore ordered target-to-source. If unreachable, the
// path is empty. SearchResult records its length in nodes, queue removals as
// expanded_nodes, and examined neighbor entries as examined_edges.
// source and target must be valid node IDs in G.
SearchResult bfs(const Graph& G, node source, node target) {
    SearchResult result;

    std::queue<node> source_queue;
    std::set<node> visited; // A array or sth like that would be better than a set. 
    std::map<node, node> discovery;

    source_queue.push(source); 
    visited.insert(source);

    while (source_queue.size() > 0) {
        node v = source_queue.front();
        source_queue.pop();
        ++result.expanded_nodes;
        if (v == target) break; 
        for (node neighbour: G.neighbors(v)) {
            ++result.examined_edges;
            if (visited.count(neighbour) > 0) continue;
            
            visited.insert(neighbour);
            source_queue.push(neighbour);
            discovery[neighbour] = v;
        }
    }

    if (visited.count(target) == 0) {
        return result;
    }

    std::vector<node> shortest_Path;
    shortest_Path.push_back(target);
    node current = target; 
    while (current != source) {
        shortest_Path.push_back(discovery[current]);
        current = discovery[current];
    }
    result.path = std::move(shortest_Path);
    result.path_length = result.path.size();
    return result;
}

// Find a shortest path by searching from both endpoints, alternating complete
// source and target BFS layers. Each side has its own visited set and parent
// map. Stop when a newly discovered node has already been found by the other
// side, then join the parent chains into a source-to-target path. If a search
// frontier is exhausted without a meeting, return an empty path. SearchResult
// counts nodes in the path and combines queue removals and examined neighbor
// entries from both directions. This alternates layers without work balancing.
// source and target must be valid node IDs in G.
SearchResult bibfs(const Graph& G, node source, node target) {
    SearchResult result;
    std::queue<node> source_queue;
    std::queue<node> target_queue;
    std::set<node> source_visited; 
    std::set<node> target_visited; 
    std::map<node, node> source_parent;
    std::map<node, node> target_parent;

    node meeting; 
    bool found = false; 

    if (source == target) {
        result.path = {source};
        result.path_length = 1;
        return result;
    }

    source_queue.push(source); 
    source_visited.insert(source);

    target_queue.push(target); 
    target_visited.insert(target);

    while (!source_queue.empty() && !target_queue.empty()) {
        // Expand one complete source-side layer.
        const std::size_t source_layer_size = source_queue.size();

        for (std::size_t i = 0; i < source_layer_size && !found; ++i) {
            node source_v = source_queue.front();
            source_queue.pop();
            ++result.expanded_nodes;

            for (node neighbour : G.neighbors(source_v)) {
                ++result.examined_edges;

                if (source_visited.count(neighbour) > 0) continue;

                source_visited.insert(neighbour);
                source_parent[neighbour] = source_v;
                source_queue.push(neighbour);

                if (target_visited.count(neighbour) > 0) {
                    meeting = neighbour;
                    found = true;
                    break;
                }
            }
        }

        if (found) break;

        // If this side is exhausted without overlap, no path exists.
        if (source_queue.empty()) break;

        // Expand one complete target-side layer.
        const std::size_t target_layer_size = target_queue.size();

        for (std::size_t i = 0; i < target_layer_size && !found; ++i) {
            node target_v = target_queue.front();
            target_queue.pop();
            ++result.expanded_nodes;

            for (node neighbour : G.neighbors(target_v)) {
                ++result.examined_edges;

                if (target_visited.count(neighbour) > 0) continue;

                target_visited.insert(neighbour);
                target_parent[neighbour] = target_v;
                target_queue.push(neighbour);

                if (source_visited.count(neighbour) > 0) {
                    meeting = neighbour;
                    found = true;
                    break;
                }
            }
        }

        if (found) break;
    }

    if (!found) {
        return result;
    }

    std::vector<node> path;

    node current = meeting;
    path.push_back(current);

    while (current != source) {
        current = source_parent.at(current);
        path.push_back(current);
    }

    std::reverse(path.begin(), path.end());

    current = meeting;

    while (current != target) {
        current = target_parent.at(current);
        path.push_back(current);
    }

    result.path = std::move(path);
    result.path_length = result.path.size();
    return result;
}

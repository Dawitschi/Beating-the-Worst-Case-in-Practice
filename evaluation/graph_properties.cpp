// AI-generated to get a quick first look at the graph data and get to know R.
// Uses the framework's Graph for loading, degrees, neighbors and edge lookup.
#include "framework/graph.hpp"
#include "framework/component_structure.hpp"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace fs = std::filesystem;

// BFS-based distance in edges, used only to calculate graph locality, not timings.
// Unlike bfs(), this returns only a distance and does not construct a path.
// With skip_edge=true, ignore the source-target edge without modifying G:
// the remaining shortest path is its detour; -1 means no path (a bridge).
int shortest_path_distance(const Graph& G, node source, node target, bool skip_edge = false) {
    if (source == target) return 0;
    std::vector<int> from_source(G.n(), -1), from_target(G.n(), -1);
    std::vector<node> source_layer{source}, target_layer{target};
    from_source[source] = from_target[target] = 0;

    auto expand_layer = [&](std::vector<node>& layer,
                            std::vector<int>& distances,
                            const std::vector<int>& other_distances) {
        std::vector<node> next_layer;
        for (node u : layer) {
            for (node v : G.neighbors(u)) {
                if (skip_edge && ((u == source && v == target) ||
                                  (u == target && v == source))) continue;
                if (distances[v] != -1) continue;
                distances[v] = distances[u] + 1;
                if (other_distances[v] != -1) {
                    return distances[v] + other_distances[v];
                }
                next_layer.push_back(v);
            }
        }
        layer = std::move(next_layer);
        return -1;
    };

    while (!source_layer.empty() && !target_layer.empty()) {
        int length;
        if (source_layer.size() <= target_layer.size()) {
            length = expand_layer(source_layer, from_source, from_target);
        } else {
            length = expand_layer(target_layer, from_target, from_source);
        }
        if (length != -1) return length;
    }
    return -1;
}

// Values with no definition (e.g. locality for a tree) are written as R's NA.
void write_number(std::ostream& out, double value) {
    if (std::isnan(value)) out << "NA";
    else if (std::isinf(value)) out << (value < 0 ? "-Inf" : "Inf");
    else out << value;
}

int main() {
    const fs::path graph_directory = "instances/graphs";
    const fs::path output_file = "evaluation/plots/graph-summary.csv";
    const fs::path temporary_file = "evaluation/plots/graph-summary.csv.tmp";
    try {
        std::vector<fs::path> files;
        for (const auto& entry : fs::directory_iterator(graph_directory)) {
            if (entry.is_regular_file() && entry.path().extension() == ".gr") {
                files.push_back(entry.path());
            }
        }
        std::sort(files.begin(), files.end());
        if (files.empty()) throw std::runtime_error("No .gr files found");

        fs::create_directories(output_file.parent_path());
        std::ofstream out(temporary_file);
        if (!out) throw std::runtime_error("Cannot open output CSV");
        out << std::setprecision(15)
            << "graph,nodes,edges,average_degree,max_degree,degree_sd,heterogeneity,"
               "degree_locality,distance_locality,locality,edge_samples,nonedge_samples,seed\n";

        const unsigned sample_count = 256;
        const double missing = std::numeric_limits<double>::quiet_NaN();
        for (std::size_t i = 0; i < files.size(); ++i) {
            Graph G(files[i]);
            if (G.n() == 0) throw std::runtime_error("Empty graph: " + files[i].string());
            ComponentStructure components(G);
            if (components.nr_components() != 1) {
                throw std::runtime_error("Expected a connected graph: " + files[i].string());
            }

            double average_degree = 2.0 * G.m() / G.n();
            unsigned max_degree = 0;
            double variance = 0;
            std::vector<std::pair<node, node>> edges;
            for (node u = 0; u < G.n(); ++u) {
                max_degree = std::max(max_degree, G.degree(u));
                variance += std::pow(G.degree(u) - average_degree, 2) / G.n();
                for (node v : G.neighbors(u)) {
                    if (u < v) edges.emplace_back(u, v);
                }
            }
            double degree_sd = std::sqrt(variance);
            double heterogeneity = average_degree > 0 ?
                std::log10(degree_sd / average_degree) : missing;

            unsigned seed = 42;
            for (unsigned char c : files[i].filename().string()) seed = seed * 33 + c;
            std::mt19937 rng(seed);
            double degree_locality = missing, distance_locality = missing;
            unsigned sampled_edges = 0, sampled_nonedge_pairs = 0;

            // Graph removes self-loops and duplicate edges, but keeps cycles
            // such as u-v-w-u. A connected graph is a tree iff m=n-1.
            // Trees have no non-bridge
            // edges; complete graphs have no nonedges. Locality is undefined.
            bool has_cycles = G.m() > G.n() - 1;
            bool has_nonedges = 2.0 * G.m() < double(G.n()) * (G.n() - 1);
            if (has_cycles && has_nonedges) {
                std::uniform_int_distribution<std::size_t> pick_edge(0, edges.size() - 1);
                double degree_sum = 0, detour_sum = 0;
                for (unsigned attempt = 0; attempt < sample_count * 100 &&
                     sampled_edges < sample_count; ++attempt) {
                    auto [u, v] = edges[pick_edge(rng)];
                    int detour = shortest_path_distance(G, u, v, true);
                    if (detour == -1) continue;
                    if (G.degree(u) > G.degree(v)) std::swap(u, v);
                    unsigned common = 0;
                    for (node w : G.neighbors(u)) {
                        if (G.has_edge(v, w)) ++common;
                    }
                    degree_sum += double(common) / (G.degree(u) - 1);
                    detour_sum += detour;
                    ++sampled_edges;
                }

                std::uniform_int_distribution<node> pick_node(0, G.n() - 1);
                double nonedge_distance_sum = 0;
                for (unsigned j = 0; j < sample_count; ++j) {
                    node u, v;
                    do { u = pick_node(rng); v = pick_node(rng); }
                    while (u == v || G.has_edge(u, v));
                    nonedge_distance_sum += shortest_path_distance(G, u, v);
                    ++sampled_nonedge_pairs;
                }
                if (sampled_edges > 0) {
                    degree_locality = degree_sum / sampled_edges;
                    double mean_distance = nonedge_distance_sum / sampled_nonedge_pairs;
                    distance_locality = mean_distance == 2 ? 0 : std::max(0.0,
                        1 - (detour_sum / sampled_edges - 2) / (mean_distance - 2));
                }
            }

            std::string name = files[i].filename().string();
            out << '"';
            for (char c : name) { if (c == '"') out << '"'; out << c; }
            out << '"' << ',' << G.n() << ',' << G.m() << ',' << average_degree
                << ',' << max_degree << ',' << degree_sd << ',';
            write_number(out, heterogeneity); out << ',';
            write_number(out, degree_locality); out << ',';
            write_number(out, distance_locality); out << ',';
            write_number(out, (degree_locality + distance_locality) / 2);
            out << ',' << sampled_edges << ',' << sampled_nonedge_pairs << ',' << seed << '\n';
            if ((i + 1) % 50 == 0) {
                std::cout << "Graph properties: " << i + 1 << '/' << files.size() << std::endl;
            }
        }
        out.close();
        if (!out) throw std::runtime_error("CSV write failed");
        // Keep the previous summary if processing fails partway through.
        fs::rename(temporary_file, output_file);
        std::cout << "Saved " << files.size() << " graph summaries to "
                  << output_file << std::endl;
    } catch (const std::exception& error) {
        std::error_code ignored;
        fs::remove(temporary_file, ignored);
        std::cerr << error.what() << '\n';
        return 1;
    }
}

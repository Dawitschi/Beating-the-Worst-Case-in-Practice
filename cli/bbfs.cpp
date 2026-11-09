#include <CLI11.hpp>

#include "framework/bbfs.hpp"
#include "framework/graph.hpp"
#include "framework/timer.hpp"

#include <algorithm>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

namespace fs = std::filesystem;

int main(int argc, char** argv) {
    CLI::App app{"Compare BFS and bidirectional BFS across all the graphs"};

    std::string graph_directory;
    std::string output_file = "results.csv";
    unsigned sample_count = 0;
    std::uint32_t seed = 42;

    app.add_option("graph_directory", graph_directory,
                   "Directory containing .gr graph files")
        ->required()
        ->check(CLI::ExistingDirectory);

    app.add_option("-n,--nodes", sample_count,
                   "Number of distinct nodes sampled per graph")
        ->required()
        ->check(CLI::Range(2u, 1000000000u));

    app.add_option("-o,--output", output_file, "Output CSV filename");
    app.add_option("--seed", seed, "Random seed");

    CLI11_PARSE(app, argc, argv);

    try {
        std::vector<fs::path> graphs;

        for (const auto& entry :
             fs::recursive_directory_iterator(graph_directory)) {
            if (entry.is_regular_file() &&
                entry.path().extension() == ".gr") {
                graphs.push_back(entry.path());
            }
        }

        std::sort(graphs.begin(), graphs.end());

        if (graphs.empty()) {
            throw std::runtime_error("No .gr files found");
        }

        std::ofstream file(output_file);
        if (!file) {
            throw std::runtime_error("Could not open output CSV");
        }

        file << std::setprecision(12);
        file << "graph,graph_nodes,graph_edges,sampled_nodes,seed,"
                "source,target,algorithm,load_ms,search_ms,found,"
                "path_length,expanded_nodes,examined_edges,path\n";

        // Create once: these timers accumulate across measurements.
        Timer::create_timer("experiment_load");
        Timer::create_timer("experiment_search");

        std::mt19937 rng(seed);

        for (std::size_t index = 0; index < graphs.size(); ++index) {

            // Load the graph 
            const fs::path& graph_file = graphs[index];

            std::cout << "[" << index + 1 << "/" << graphs.size()
                      << "] " << graph_file.filename().string()
                      << std::endl;

            const double load_before =
                Timer::time("experiment_load").count();

            Timer::start_timer("experiment_load");
            Graph G(graph_file);
            Timer::stop_timer("experiment_load");

            const double load_ms =
                Timer::time("experiment_load").count() - load_before;

            const std::size_t count =
                std::min<std::size_t>(sample_count, G.n());

            if (count < 2) {
                std::cerr << "Skipping: fewer than two nodes\n";
                continue;
            }

            // Sample the node pairs
            std::vector<node> sampled(G.n());
            std::iota(sampled.begin(), sampled.end(), node{0});

            for (std::size_t i = 0; i < count; ++i) {
                std::uniform_int_distribution<std::size_t> pick(
                    i, sampled.size() - 1);
                std::swap(sampled[i], sampled[pick(rng)]);
            }

            sampled.resize(count);

            const std::string graph_name =
                graph_file.lexically_relative(
                    fs::path(graph_directory)).generic_string();

            std::string graph_csv = "\"";
            for (char c : graph_name) {
                if (c == '"') graph_csv += '"';
                graph_csv += c;
            }
            graph_csv += '"';

            // Run the measurements. 
            using SearchFunction =
                SearchResult (*)(const Graph&, node, node);

            auto run_and_write = [&](const char* algorithm,
                                     SearchFunction search,
                                     node source, node target) {
                const double search_before =
                    Timer::time("experiment_search").count();

                Timer::start_timer("experiment_search");
                SearchResult result = search(G, source, target);
                Timer::stop_timer("experiment_search");

                const double search_ms =
                    Timer::time("experiment_search").count()
                    - search_before;

                file << graph_csv
                     << "," << G.n()
                     << "," << G.m()
                     << "," << count
                     << "," << seed
                     << "," << source
                     << "," << target
                     << "," << algorithm
                     << "," << load_ms
                     << "," << search_ms
                     << "," << (result.path.empty() ? 0 : 1)
                     << "," << result.path_length
                     << "," << result.expanded_nodes
                     << "," << result.examined_edges
                     << ",\"";

                for (std::size_t i = 0; i < result.path.size(); ++i) {
                    if (i != 0) file << " ";
                    file << result.path[i];
                }

                file << "\"\n";

                if (!file) {
                    throw std::runtime_error("CSV write failed");
                }
            };

            for (std::size_t i = 0; i < count; ++i) {
                for (std::size_t j = i + 1; j < count; ++j) {
                    const node source = sampled[i];
                    const node target = sampled[j];
                    run_and_write("bibfs", bibfs, source, target);
                    run_and_write("bfs", bfs, source, target);
                }
            }

            file.flush();
            if (!file) {
                throw std::runtime_error("CSV flush failed");
            }
        }

        file.close();
        if (!file) {
            throw std::runtime_error("Could not finish writing CSV");
        }

        std::cout << "Results saved to " << output_file << "\n";
    } catch (const std::exception& error) {
        std::cerr << "Experiment failed: " << error.what() << "\n";
        return 1;
    }

    return 0;
}
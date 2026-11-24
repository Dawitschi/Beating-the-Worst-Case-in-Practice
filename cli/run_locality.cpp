#include <CLI11.hpp>
#include <random>
#include <vector>

#include "framework/locality.hpp"
#include "framework/graph.hpp"
namespace fs = std::filesystem;

int main(int argc, char **argv) {
    CLI::App app{"Locality options"};

    std::string graph_directory;
    std::string output_file = "instances/locality/locality.csv";
    unsigned sample_count = 0;
    std::uint32_t seed = 42;

    app.add_option("graph_directory", graph_directory,
                   "Directory containing .gr graph files")
            ->required()
            ->check(CLI::ExistingDirectory);

    app.add_option("-o,--output", output_file, "Output CSV filename");

    app.add_option("-n,--nodes", sample_count,
                   "Number of distinct nodes sampled per graph")
            ->required()
            ->check(CLI::Range(2u, 1000000000u));

    app.add_option("--seed", seed, "Random seed");


    CLI11_PARSE(app, argc, argv);

    try {
        std::vector<fs::path> graphs;

        for (const auto &entry:
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

        std::vector<Locality *> locality;

        //TODO build all locality classes
        auto *c = new Density();
        locality.push_back(c);

        auto *closeness = new ClosenessLocality();
       locality.push_back(closeness);

        file << std::setprecision(12);
        file << "graph,graph_nodes,graph_edges,"
                "localityMeasure, locality,number_of_nodes\n";

        std::cout << "Now starting to calculate..."<< std::endl;
        for (auto &loc: locality) {
            for (std::size_t index = 0; index < graphs.size(); ++index) {
                // Load the graph
                const fs::path &graph_file = graphs[index];
                Graph G(graph_file);

                // provides the number of nodes to samplte  for approximation algos
                const std::size_t count =
                        std::min<std::size_t>(sample_count, G.n());

                if (count < 2) {
                    std::cerr << "Skipping: fewer than two nodes\n";
                    continue;
                }
                const std::string graph_name =
                        graph_file.lexically_relative(
                            fs::path(graph_directory)).generic_string();

            std::cout << "[" << index + 1 << "/" << graphs.size()
                      << "] " << graph_file.filename().string()
                      << std::endl;

            std::string graph_csv = "\"";
            for (char c : graph_name) {
                if (c == '"') graph_csv += '"';
                graph_csv += c;
            }
            graph_csv += '"';

                file << graph_csv
                        << "," << G.n()
                        << "," << G.m()
                        << "," << loc->localityName()
                        << "," << loc->computeLocality(G,count, seed)
                        << "," << count
                        << ",\n";

                if (!file) {
                    throw std::runtime_error("CSV write failed");
                }
                file.flush();
                if (!file) {
                    throw std::runtime_error("CSV flush failed");
                }
            }
        }

        file.close();
        if (!file) {
            throw std::runtime_error("Could not finish writing CSV");
        }
        std::cout << "Results saved to " << output_file << "\n";
    } catch (const std::exception &error) {
        std::cerr << "Experiment failed: " << error.what() << "\n";
        return 1;
    }
    return 0;
}

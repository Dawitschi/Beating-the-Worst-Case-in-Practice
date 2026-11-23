#include <CLI11.hpp>
#include <random>
#include <vector>

#include "framework/locality.hpp"
#include "framework/graph.hpp"
namespace fs = std::filesystem;

int main(int argc, char** argv) {
    CLI::App app{"Locality options"};

    std::string graph_directory;
    std::string output_file = "test.csv";
    unsigned sample_count = 0;

    app.add_option("graph_directory", graph_directory,
                   "Directory containing .gr graph files")
        ->required()
        ->check(CLI::ExistingDirectory);

    app.add_option("-o,--output", output_file, "Output CSV filename");

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

        std::vector<CoreCentrality*> locality;

        //TODO build all locality classes
        auto *c = new CoreCentrality();

        locality.push_back(c);
        file << std::setprecision(12);
        file << "graph,graph_nodes,graph_edges,"
                "localityMeasure, locality\n";
        
        for(auto &loc: locality){
        for (std::size_t index = 0; index < graphs.size(); ++index) {
            std::uint32_t seed = 42;
            // Load the graph
            const fs::path& graph_file = graphs[index];
            Graph G(graph_file);


            const std::string graph_name =
                graph_file.lexically_relative(
                    fs::path(graph_directory)).generic_string();

            std::string graph_csv = "\"";
            for (char c : graph_name) {
                if (c == '"') graph_csv += '"';
                graph_csv += c;
            }
            graph_csv += '"';

            file << graph_csv
                 << "," << G.n()
                 << "," << G.m()
                 << "," << c->localityName()
                 << "," << c->computeLocality(G)
                 <<",\n";

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

    }catch(const std::exception& error) {
        std::cerr << "Experiment failed: " << error.what() << "\n";
        return 1;
    }
    return 0;
 }

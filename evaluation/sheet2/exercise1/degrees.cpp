#include "framework/graph.hpp"
#include "framework/component_structure.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <stdexcept>
#include <system_error>
#include <unordered_set>
#include <vector>

namespace fs = std::filesystem;

int main() {
    const fs::path graph_directory = "instances/graphs";
    const fs::path output_file = "evaluation/sheet2/exercise1/degree-samples.csv";
    const fs::path temporary_file = "evaluation/sheet2/exercise1/graph-summary.csv.tmp";

    try
    {
        std::vector<fs::path> files;
        for (const auto& entry : fs::directory_iterator(graph_directory)) {
            if (entry.is_regular_file() && entry.path().extension() == ".gr") {
                files.push_back(entry.path());
            }
        }
        std::sort(files.begin(), files.end());
        fs::create_directories(output_file.parent_path());
        std::ofstream out(temporary_file);
        if (!out) throw std::runtime_error("Cannot open output CSV");
        out << std::setprecision(15);
        out << "graph, seed, degrees, secondDegrees\n";

        const unsigned seed = 42;
        std::mt19937 rng(seed);

        for (std::size_t i = 0; i < files.size(); ++i) {
            Graph G(files[i]);
            const unsigned sample_count = std::min<std::size_t>(512, G.n());
            if (G.n() == 0) throw std::runtime_error("Empty graph: " + files[i].string());
            ComponentStructure components(G);
            if (components.nr_components() != 1) {
                throw std::runtime_error("Expected a connected graph: " + files[i].string());
            }
            out << files[i].filename().string() << ','
                << seed << ",\"";
            std::uniform_int_distribution<node> pick_node(0, G.n() - 1);
            std::unordered_set<node> seen;

            std::vector<node> degree; 
            std::vector<node> secondDegree; 

            while (seen.size() < sample_count) {
                const node v = pick_node(rng);
                if (seen.insert(v).second) {
                    degree.push_back(G.neighbors(v).size());
                    unsigned sum = 0; 
                    for (node neighbour : G.neighbors(v)) {
                        sum += G.neighbors(neighbour).size();
                    }
                    secondDegree.push_back(sum);
                }
            }

            for (unsigned n : degree) {
                out << n << ' ';
            }
            out << "\",\"";
            for (node n : secondDegree) {
                out << n << ' ';
            }
            out << "\"\n";
            std::cout << (i + 1) << '/' << files.size() << " graphs" << std::endl;
        }

        out.close();
        if (!out) throw std::runtime_error("CSV write failed");
        fs::rename(temporary_file, output_file);
        std::cout << "Saved " << files.size() << " graph summaries to "
                  << output_file << std::endl;
    }
    catch(const std::exception& e)
    {
        std::error_code ignored;
        fs::remove(temporary_file, ignored);
        std::cerr << e.what() << '\n';
        return 1;
    }    

}

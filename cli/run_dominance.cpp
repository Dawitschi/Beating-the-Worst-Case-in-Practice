#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>
#include <filesystem>
#include "CLI11.hpp"
#include "framework/component_structure.hpp"
#include "framework/graph.hpp"
#include "framework/timer.hpp"
#include "framework/dominance.hpp"

namespace fs = std::filesystem;

int main(int argc, char** argv) {
    CLI::App app{"Beating the Worst Case - Dominance Rule"};

    std::string input_path;
    app.add_option("input_path", input_path, "Path to a .gr file or a directory containing .gr files")
       ->required()
       ->check(CLI::ExistingPath);

    CLI11_PARSE(app, argc, argv);

    // Sorted Batch Processing with  Logging
    if (fs::is_directory(input_path)) {
        std::cout << "Starting scan and logging: " << input_path << "\n\n";

     
        std::vector<fs::path> graph_files;
        for (const auto& entry : fs::directory_iterator(input_path)) {
            if (entry.path().extension() == ".gr") {
                graph_files.push_back(entry.path());
            }
        }

        // Sort files numerically 
        std::sort(graph_files.begin(), graph_files.end(), [](const fs::path& a, const fs::path& b) {
            try {
                return std::stoi(a.stem().string()) < std::stoi(b.stem().string());
            } catch (...) {
                return a.filename().string() < b.filename().string();
            }
        });

        // Open log file 
        std::ofstream csv_file("dominance_log.csv", std::ios::out);
        csv_file << "filename,orig_nodes,orig_edges,rem_nodes,rem_edges,largest_comp_size,time_total,time_load,time_dominance,time_component\n";


        Timer::create_timer("total_time");
        Timer::create_timer("load_graph");
        Timer::create_timer("dominance");
        Timer::create_timer("components");

        double prev_total = 0.0;
        double prev_load = 0.0;
        double prev_dom = 0.0;
        double prev_comp = 0.0;

        int processed_count = 0;

        for (const auto& path : graph_files) {
            std::string file_path = path.string();
            std::string filename = path.filename().string();

            try {
                Timer::start_timer("total_time");

                Timer::start_timer("load_graph");
                Graph G(file_path);
                Timer::stop_timer("load_graph");

                Timer::start_timer("dominance");
                Graph R = reduce_dominance(G);
                Timer::stop_timer("dominance");

                Timer::start_timer("components");
                ComponentStructure CS(R);
                Timer::stop_timer("components");

                Timer::stop_timer("total_time");

                unsigned largest_comp_size = 0;
                if (CS.nr_components() > 0) {
                    largest_comp_size = CS.n(CS.largest_component());
                }

                // Calculate exact time spent on the current graph using deltas
                double cur_total = Timer::time("total_time").count();
                double cur_load = Timer::time("load_graph").count();
                double cur_dom = Timer::time("dominance").count();
                double cur_comp = Timer::time("components").count();

                double total_ms = cur_total - prev_total;
                double load_ms = cur_load - prev_load;
                double dom_ms = cur_dom - prev_dom;
                double comp_ms = cur_comp - prev_comp;

                prev_total = cur_total;
                prev_load = cur_load;
                prev_dom = cur_dom;
                prev_comp = cur_comp;

                csv_file << filename << ","
                         << G.n() << "," << G.m() << ","
                         << R.n() << "," << R.m() << ","
                         << largest_comp_size << ","
                         << total_ms << "," << load_ms << "," << dom_ms << "," << comp_ms << "\n";
                csv_file.flush();

            
                processed_count++;
                std::cout << "Processed (" << processed_count << "/" << graph_files.size() << "): "
                          << filename << " -> Kernel: " << largest_comp_size 
                          << " (" << total_ms << " ms)      \r" << std::flush;

            } catch (const std::exception& e) {
                std::cerr << "\nError processing [" << filename << "]: " << e.what() << "\n";
            }
        }

        csv_file.close();

        std::cout << "\n\nBatch processing completed.\n";
        std::cout << "Total processed graphs: " << processed_count << "\n";

    } 
    //For single File
    else if (fs::is_regular_file(input_path)) {
        Timer::create_timer("total_time");
        Timer::create_timer("load_graph");
        Timer::create_timer("dominance");
        Timer::create_timer("components");

        Timer::start_timer("total_time");
        
        Timer::start_timer("load_graph");
        Graph G(input_path);
        Timer::stop_timer("load_graph");

        Timer::start_timer("dominance");
        Graph R = reduce_dominance(G);
        Timer::stop_timer("dominance");

        Timer::start_timer("components");
        ComponentStructure CS(R);
        Timer::stop_timer("components");

        Timer::stop_timer("total_time");

        unsigned largest_comp_size = 0;
        if (CS.nr_components() > 0) {
            largest_comp_size = CS.n(CS.largest_component());
        }

        std::cout << "filename: " << fs::path(input_path).filename().string() << std::endl;
        std::cout << "orig_nodes: " << G.n() << std::endl;
        std::cout << "orig_edges: " << G.m() << std::endl;
        std::cout << "rem_nodes: " << R.n() << std::endl;
        std::cout << "rem_edges: " << R.m() << std::endl;
        std::cout << "largest_comp_size: " << largest_comp_size << std::endl;
       
        std::cout << "\ntime report:" << std::endl;
        for (const auto& timer_name : Timer::existing_timers()) {
            std::cout << timer_name << ":\t" << Timer::time(timer_name).count() << " ms" << std::endl;
        }
    }

    return 0;
}
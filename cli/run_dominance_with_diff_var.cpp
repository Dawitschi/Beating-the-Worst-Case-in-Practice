#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>
#include <filesystem>
#include "CLI11.hpp"
#include "framework/graph.hpp"
#include "framework/timer.hpp"
#include "framework/dominance.hpp"
#include "framework/dominance_with_diff_var.hpp"

namespace fs = std::filesystem;

int main(int argc, char** argv) {
    CLI::App app{"Variants"};

    std::string input_path;
    app.add_option("input_path", input_path)->required()->check(CLI::ExistingPath);

    CLI11_PARSE(app, argc, argv);

    if (fs::is_directory(input_path)) {
        std::vector<fs::path> graph_files;
        for (const auto& entry : fs::directory_iterator(input_path)) {
            if (entry.path().extension() == ".gr") {
                graph_files.push_back(entry.path());
            }
        }

        std::sort(graph_files.begin(), graph_files.end(), [](const fs::path& a, const fs::path& b) {
            try {
                return std::stoi(a.stem().string()) < std::stoi(b.stem().string());
            } catch (...) {
                return a.filename().string() < b.filename().string();
            }
        });

        std::ofstream csv_file("variants_log.csv", std::ios::out);
        csv_file << "filename,orig_nodes,orig_edges,time_array,time_hash,time_sorted\n";

        Timer::create_timer("Array");
        Timer::create_timer("Hash");
        Timer::create_timer("Sorted");

        double prev_arr = 0.0, prev_hash = 0.0, prev_sorted = 0.0;

        for (const auto& path : graph_files) {
            std::string file_path = path.string();
            std::string filename = path.filename().string();

            try {
                Graph G(file_path);

                Timer::start_timer("Array");
                Graph R1 = reduce_dominance(G);
                Timer::stop_timer("Array");

                Timer::start_timer("Hash");
                Graph R2 = reduce_dominance_hash(G);
                Timer::stop_timer("Hash");

                Timer::start_timer("Sorted");
                Graph R3 = reduce_dominance_sorted(G);
                Timer::stop_timer("Sorted");

                double cur_arr = Timer::time("Array").count();
                double cur_hash = Timer::time("Hash").count();
                double cur_sorted = Timer::time("Sorted").count();

                double t_arr = cur_arr - prev_arr;
                double t_hash = cur_hash - prev_hash;
                double t_sorted = cur_sorted - prev_sorted;

                prev_arr = cur_arr;
                prev_hash = cur_hash;
                prev_sorted = cur_sorted;

                csv_file << filename << ","
                         << G.n() << "," << G.m() << ","
                         << t_arr << "," << t_hash << "," << t_sorted << "\n";
                csv_file.flush();

                std::cout << "Logged: " << filename << "\r" << std::flush;

            } catch (const std::exception& e) {
                std::cerr << "\nError: " << filename << "\n";
            }
        }
        csv_file.close();
        std::cout << "\nVariants finished.\n";
    }

    return 0;
}
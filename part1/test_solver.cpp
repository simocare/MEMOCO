#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <cstdlib>  // For system()
#include <sstream>
#include <string>

// Define test cases (number of holes)
std::vector<int> hole_sizes = {5, 10, 15, 20, 25, 30, 35, 40, 45, 50};
std::string file_name = "results.csv";

int main() {
    std::ofstream results(file_name);
    if (!results) {
        std::cerr << "Error: Could not open results.csv for writing.\n";
        return 1;
    }

    results << "Board Filename,Number of Holes,Solving Time (seconds)\n"; // CSV header

    for (size_t i = 0; i < hole_sizes.size(); ++i) {
        int num_holes = hole_sizes[i];
        std::ostringstream board_filename;
        board_filename << "board_" << num_holes << ".dat";  // e.g., board_5.dat, board_10.dat
        std::string board_file = board_filename.str();

        std::cout << "Testing with " << num_holes << " holes (" << board_file << ")...\n";

        // Generate board with `num_holes` and unique filename
        std::string generate_cmd = "./generate_board.out 10 " + std::to_string(num_holes) + " " + board_file;
        if (system(generate_cmd.c_str()) != 0) {
            std::cerr << "Error running generate_board.out\n";
            continue;
        }

        // Start timing the CPLEX solver
        auto start = std::chrono::high_resolution_clock::now();

        // Run the solver with the generated board file
        std::string run_cmd = "./main.out " + board_file + " > temp_output.txt 2>&1"; // Redirect output to temp file
        if (system(run_cmd.c_str()) != 0) {
            std::cerr << "Error running main.out\n";
            continue;
        }

        // Stop timing
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end - start;
        double solving_time = elapsed.count();

        std::cout << "Solving time: " << solving_time << " sec\n\n";

        // Save result
        results << board_file << "," << num_holes << "," << solving_time << "\n";
    }

    results.close();
    std::cout << "Results saved to " << file_name << "\n";
    return 0;
}

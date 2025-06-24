#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <set>
#include <stdexcept>

#include "part1/generate_board.h"

#include "part2/TSPSolver.h"
#include "part2/TSP.h"       

const std::string OUTPUT_DIR = "parameter_tuning";
const std::string CSV_FILENAME = "tuning_results.csv";

const std::vector<int> SIZES = {5, 10, 15, 20, 30};
const std::vector<double> DENSITIES = {0.05, 0.1, 0.15, 0.2};
const int REPEATS = 3;

//const std::vector<double> ALPHAS = {0.5, 0.75, 1.0}; 1)
//const std::vector<double> BETAS = {0.3, 0.5, 0.7};   1)

// const std::vector<double> ALPHAS = {0.15, 0.35, 0.5};
// const std::vector<double> BETAS = {0.1, 0.3, 0.5};

//24 / 06 -> partendo da 1) spostiamo un po' i valori di alpha e beta verso il basso
const std::vector<double> ALPHAS = {0.35, 0.5, 0.75};
const std::vector<double> BETAS = {0.2, 0.3, 0.5};

const std::vector<double> DECAY_FACTORS = {0.85, 0.9, 0.95};
const std::vector<double> LAMBDAS = {0.0, 0.01, 0.05};

struct TuningResult {
    int size;
    double density;
    int holes;
    int repeat;
    double alpha;
    double beta;
    double decayFactor;
    double lambda;
    double final_cost;
    double time_sec;
    std::string board_filename;
};

bool file_exists(const std::string& name) {
    std::ifstream f(name.c_str());
    return f.good();
}

std::string combine_path(const std::string& dir, const std::string& filename) {
    if (!dir.empty() && dir.back() != '/') {
        return dir + "/" + filename;
    }
    return dir + filename;
}

int main(int argc, char* argv[]) {
    bool save_logs = false;
    if (argc > 1 && std::string(argv[1]) == "--save-logs") {
        save_logs = true;
    }

    srand(static_cast<unsigned int>(time(nullptr)));

    std::string mkdir_command = "mkdir -p " + OUTPUT_DIR;
    int mkdir_result = std::system(mkdir_command.c_str());
    if (mkdir_result != 0) {
        std::cerr << "Error creating directory " << OUTPUT_DIR << std::endl;
        return 1;
    }

    std::ofstream csv_file;
    bool first_write = !file_exists(CSV_FILENAME);
    csv_file.open(CSV_FILENAME, std::ios_base::app);

    if (!csv_file.is_open()) {
        std::cerr << "Error: Could not open CSV file: " << CSV_FILENAME << std::endl;
        return 1;
    }

    if (first_write) {
        csv_file << "size,density,holes,repeat,alpha,beta,decayFactor,lambda,final_cost,time_sec,board_filename\n";
    }

    for (int size : SIZES) {
        int total_cells = size * size;
        for (double density : DENSITIES) {
            int num_holes = static_cast<int>(total_cells * density);

            if (num_holes < 3 || num_holes > 0.6 * total_cells) {
                continue;
            }

            for (int r = 0; r < REPEATS; ++r) {
                std::string original_board_fname = combine_path(OUTPUT_DIR, "board_" + std::to_string(size) + "_" + std::to_string(num_holes) + "_" + std::to_string(r) + ".dat");
                
                generateBoard(size, num_holes, original_board_fname);
                
                if (!file_exists(original_board_fname)) {
                    std::cerr << "Error: Board file " << original_board_fname << " was not created. Skipping." << std::endl;
                    continue;
                }

                TuningResult best_result;
                best_result.final_cost = std::numeric_limits<double>::infinity();

                int idx = 0;
                for (double alpha : ALPHAS) {
                    for (double beta : BETAS) {
                        for (double decay_factor : DECAY_FACTORS) {
                            for (double lambda : LAMBDAS) {
                                std::string current_log_fname = combine_path(OUTPUT_DIR, "board_" + std::to_string(size) + "_" + std::to_string(num_holes) + "_" + std::to_string(r) + "_run" + std::to_string(idx) + "_log.txt");
                                idx++;

                                TSP tspInstance;
                                try {
                                    tspInstance.read(original_board_fname.c_str());
                                } catch (const std::exception& e) {
                                    std::cerr << "Error reading board file " << original_board_fname << ": " << e.what() << std::endl;
                                    if (file_exists(current_log_fname)) {
                                        std::remove(current_log_fname.c_str());
                                    }
                                    continue;
                                }

                                TSPSolution aSolution(tspInstance);

                                auto start_time = std::chrono::high_resolution_clock::now();

                                TSPSolver tspSolver(current_log_fname, alpha, beta, decay_factor, lambda);
                                tspSolver.initRnd(aSolution);

                                TSPSolution current_best_solution(tspInstance);
                                int tabuLength = 10;
                                int maxIterations = 1000;
                                tspSolver.solve(tspInstance, aSolution, tabuLength, maxIterations, current_best_solution);

                                auto end_time = std::chrono::high_resolution_clock::now();
                                std::chrono::duration<double> elapsed_seconds = end_time - start_time;
                                double time_taken = elapsed_seconds.count();

                                double current_final_cost = tspSolver.evaluate(current_best_solution, tspInstance);

                                if (current_final_cost >= 0 && current_final_cost < best_result.final_cost) {
                                    best_result = {
                                        size, density, num_holes, r,
                                        alpha, beta, decay_factor, lambda,
                                        current_final_cost, time_taken,
                                        original_board_fname
                                    };
                                }

                                if (!save_logs && file_exists(current_log_fname)) {
                                    std::remove(current_log_fname.c_str());
                                }
                            }
                        }
                    }
                }

                if (best_result.final_cost != std::numeric_limits<double>::infinity()) {
                    csv_file << best_result.size << ","
                             << best_result.density << ","
                             << best_result.holes << ","
                             << best_result.repeat << ","
                             << best_result.alpha << ","
                             << best_result.beta << ","
                             << best_result.decayFactor << ","
                             << best_result.lambda << ","
                             << std::fixed << std::setprecision(4) << best_result.final_cost << ","
                             << std::fixed << std::setprecision(4) << best_result.time_sec << ","
                             << best_result.board_filename << "\n";
                    csv_file.flush();

                    std::cout << "Best tuning result for size=" << size << ", density=" << density
                              << ", repeat=" << r << " saved. Cost: " << best_result.final_cost << std::endl;
                } else {
                    std::cout << "No valid solution found for size=" << size << ", density=" << density
                              << ", repeat=" << r << std::endl;
                }

                if (file_exists(original_board_fname)) {
                    std::remove(original_board_fname.c_str());
                }
            }
        }
    }

    csv_file.close();
    std::cout << "Parameter tuning complete. Results saved to " << CSV_FILENAME << std::endl;

    return 0;
}

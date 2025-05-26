#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <string>
#include <vector>
#include <utility>
#include <cstdlib>
#include <cstdio>
#include <chrono>
#include <array>
#include <sys/stat.h>
#include <sys/types.h>

struct Params {
    double alpha;
    double beta;
    double decayFactor;
    double lambda;
};

bool create_directory(const std::string& dir) {
    struct stat st;
    if (stat(dir.c_str(), &st) != 0) {
        if (mkdir(dir.c_str(), 0755) != 0) {
            std::cerr << "Could not create directory: " << dir << std::endl;
            return false;
        }
    } else if (!S_ISDIR(st.st_mode)) {
        std::cerr << dir << " exists but is not a directory!" << std::endl;
        return false;
    }
    return true;
}

bool file_exists(const std::string& filename) {
    struct stat buffer;
    return (stat(filename.c_str(), &buffer) == 0);
}

// Load best parameters for each (size, density)
std::map<std::pair<int, double>, Params> loadBestParams(const std::string& csv_filename) {
    std::ifstream file(csv_filename);
    std::map<std::pair<int, double>, Params> bestParams;

    if (!file.is_open()) {
        std::cerr << "Could not open parameter file: " << csv_filename << std::endl;
        return bestParams;
    }

    std::string line;
    std::getline(file, line); // skip header

    while (std::getline(file, line)) {
        std::istringstream ss(line);
        std::string token;
        std::vector<std::string> tokens;

        while (std::getline(ss, token, ',')) {
            tokens.push_back(token);
        }

        if (tokens.size() < 10) continue;

        int size = std::stoi(tokens[0]);
        double density = std::stod(tokens[1]);
        double alpha = std::stod(tokens[6]);
        double beta = std::stod(tokens[7]);
        double decay = std::stod(tokens[8]);
        double lambda = std::stod(tokens[9]);

        bestParams[{size, density}] = {alpha, beta, decay, lambda};
    }

    return bestParams;
}

// Execute command and capture stdout
std::string execCommand(const std::string& cmd) {
    std::array<char, 128> buffer;
    std::string result;

    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) return "ERROR";

    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        result += buffer.data();
    }
    pclose(pipe);

    return result;
}

int main() {
    std::vector<int> sizes = {5, 10, 15, 20, 30};
    std::vector<double> densities = {0.05, 0.1, 0.15, 0.2};
    int repeats = 3;

    std::string generator = "part1/generate_board.out";
    std::map<std::string, std::string> solvers = {
        {"tabu", "part2/main_tabu.out"},
        {"cplex", "part1/main_cplex.out"}
    };
    std::string param_csv = "summary_tuning.csv";
    std::string output_dir = "experiments";
    std::string result_csv = "benchmark_results.csv";

    if (!create_directory(output_dir)) {
        std::cerr << "Failed to create output directory, exiting." << std::endl;
        return 1;
    }

    auto bestParams = loadBestParams(param_csv);

    bool first_write = true;

    std::ofstream outfile;
    for (int size : sizes) {
        int total_cells = size * size;

        for (double density : densities) {
            int holes = static_cast<int>(total_cells * density);
            if (holes < 3 || holes > 0.6 * total_cells) continue;

            for (int r = 0; r < repeats; ++r) {
                std::string fname = output_dir + "/board_" + std::to_string(size) + "_" +
                                    std::to_string(holes) + "_" + std::to_string(r) + ".dat";

                // Generate the board
                std::string gen_cmd = generator + " " + std::to_string(size) + " " +
                                      std::to_string(holes) + " " + fname;
                if (std::system(gen_cmd.c_str()) != 0) {
                    std::cerr << "Failed to generate board: " << fname << "\n";
                    continue;
                }

                // Prepare CSV
                if (first_write) {
                    outfile.open(result_csv, std::ios::out);
                    outfile << "solver,size,density,holes,repeat,filename,final_cost,time_sec\n";
                    first_write = false;
                } else {
                    outfile.open(result_csv, std::ios::app);
                }

                for (const auto& solver : solvers) {
                    const std::string& solver_name = solver.first;
                    const std::string& solver_exec = solver.second;

                    std::string cmd = solver_exec + " " + fname;

                    if (solver_name == "tabu") {
                        auto it = bestParams.find({size, density});
                        if (it != bestParams.end()) {
                            Params p = it->second;
                            cmd += " --alpha=" + std::to_string(p.alpha) +
                                   " --beta=" + std::to_string(p.beta) +
                                   " --decayFactor=" + std::to_string(p.decayFactor) +
                                   " --lambda=" + std::to_string(p.lambda);

                            std::cout << "[DEBUG] Running 'tabu' with params: "
                                      << "alpha=" << p.alpha << ", "
                                      << "beta=" << p.beta << ", "
                                      << "decayFactor=" << p.decayFactor << ", "
                                      << "lambda=" << p.lambda << "\n";
                        } else {
                            std::cerr << "Missing parameters for tabu with size=" << size
                                      << ", density=" << density << "\n";
                            continue;
                        }
                    }

                    auto start = std::chrono::high_resolution_clock::now();
                    std::string output = execCommand(cmd);
                    auto end = std::chrono::high_resolution_clock::now();
                    std::chrono::duration<double> elapsed = end - start;

                    double final_cost = -1.0;
                    std::istringstream iss(output);
                    std::string line;
                    while (std::getline(iss, line)) {
                        if (line.rfind("FINAL_VALUE", 0) == 0) {
                            size_t pos = line.find(":");
                            if (pos != std::string::npos) {
                                final_cost = std::stod(line.substr(pos + 1));
                            }
                            break;
                        }
                    }

                    outfile << solver_name << ","
                            << size << ","
                            << density << ","
                            << holes << ","
                            << r << ","
                            << fname << ","
                            << final_cost << ","
                            << elapsed.count() << "\n";

                    std::cout << "✓ " << solver_name << " | size=" << size
                              << " density=" << density << " r=" << r
                              << " -> cost=" << final_cost
                              << " (" << elapsed.count() << "s)\n";
                }

                outfile.close();
            }
        }
    }

    std::cout << "Benchmarking complete. Results saved to " << result_csv << "\n";
    return 0;
}

#include <cstdio>
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include "cpxmacro.h"

using namespace std;

// error status and message buffer (from cpxmacro.h)
int status;
char errmsg[BUF_SIZE];


// // Data
// const int N = 4;

// // Cost matrix (NxN, not NxA)
// const double C[N][N] = { {1.0, 1.0, 1.0, 1.0},
//                          {1.0, 1.0, 1.0, 1.0},
//                          {1.0, 1.0, 1.0, 1.0},
//                          {1.0, 1.0, 1.0, 1.0} };

const int NAME_SIZE = 512;
char name[NAME_SIZE];

/*MAP FOR X, Y VARS*/
vector<vector<int>> map_x;  // x_ij ---> map_x[i][j]
vector<vector<int>> map_y;  // y_ij ---> map_y[i][j]

struct Hole {
    int x, y;
};

std::vector<Hole> readBoard(const std::string& filename) {
    std::ifstream inFile(filename);
    if (!inFile) {
        std::cerr << "Error opening file: " << filename << std::endl;
        exit(1);
    }

    int size;
    inFile >> size; // Read board size (assumed square)
    std::vector<Hole> holes;

    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            int value;
            inFile >> value;
            if (value == 1) {
                holes.push_back({x, y}); // Store hole position
            }
        }
    }

    inFile.close();
    return holes;
}


// Function to compute the cost matrix based on the hole positions
std::vector<std::vector<double>> computeCostMatrix(const std::vector<Hole>& holes) {
    int N = holes.size();
    std::vector<std::vector<double>> C(N, std::vector<double>(N, 0));

    // Compute Euclidean distances between holes
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (i != j) {
                double dx = holes[i].x - holes[j].x;
                double dy = holes[i].y - holes[j].y;
                C[i][j] = sqrt(dx * dx + dy * dy);
            }
        }
    }
    return C;
}

void setupLP(CEnv env, Prob lp, const std::vector<std::vector<double>>& C, int N) {
    int current_var_position = 0;

    /* MAP FOR x VARS */
    map_x.resize(N);
    for (int i = 0; i < N; ++i) {
        map_x[i].resize(N, -1);
    }

    // Add x vars
    for (int i = 0; i < N; i++)
    {
        for (int j = 1; j < N; j++)
        {
            if (i == j) continue;  // Skip self-loops

            char xtype = 'C'; // Fix missing semicolon
            double lb = 0.0;
            //double ub = N - 1; // x_ij <= (|N|-1)y_ij
            double ub = CPX_INFBOUND;
            snprintf(name, NAME_SIZE, "x_%d_%d", i, j);
            char* xname = (char*)(&name[0]);
            CHECKED_CPX_CALL(CPXnewcols, env, lp, 1, &C[i][j], &lb, &ub, &xtype, &xname);
            map_x[i][j] = current_var_position++;
        }
    }

    /* MAP FOR y VARS */
    map_y.resize(N);
    for (int i = 0; i < N; ++i) {
        map_y[i].resize(N, -1);
    }

    // Add y vars
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
        {
            if (i == j) continue; // Skip self-loops

            char ytype = 'B';
            double lb = 0;
            double ub = 1;
            snprintf(name, NAME_SIZE, "y_%d_%d", i, j);
            char* yname = (char*)(&name[0]);
            CHECKED_CPX_CALL(CPXnewcols, env, lp, 1, &C[i][j], &lb, &ub, &ytype, &yname);
            map_y[i][j] = current_var_position++;
            // 🔍 DEBUG: Print assigned variable index
            std::cout << "Assigned y_" << i << "_" << j << " to index " << map_y[i][j] << std::endl;
        }
    }

    // Constraints: Flow conservation
    for (int k = 1; k < N; k++)  // Fix comma error
    {
        std::vector<int> idx;
        std::vector<double> coef;

        // Incoming flow to node k: sum_{i} x_ik
        for (int i = 0; i < N; i++) {
            if (i == k) continue;
            if (map_x[i][k] >= 0) {
                idx.push_back(map_x[i][k]);
                coef.push_back(1.0);
            }
        }

        // Outgoing flow from node k (excluding node 0): sum_{j, j ≠ 0} x_kj
        for (int j = 1; j < N; j++) {
            if (j == k) continue;
            if (map_x[k][j] >= 0) {
                idx.push_back(map_x[k][j]);
                coef.push_back(-1.0);
            }
        }

        double rhs = 1.0;
        char sense = 'E';
        int matbeg = 0;

        std::cout << "Adding flow conservation constraint at node " << k << std::endl;
        CHECKED_CPX_CALL(CPXaddrows, env, lp, 0, 1, idx.size(), &rhs, &sense, &matbeg, idx.data(), coef.data(), NULL, NULL);
    }

    // Constraints: the sum of outgoing arcs from i
    for (int i = 0; i < N; i++) {
        std::vector<int> idx;
        std::vector<double> coef;

        for (int j = 1; j < N; j++) {
            if (i == j) continue;
            if (map_y[i][j] >= 0) {
                idx.push_back(map_y[i][j]);
                coef.push_back(1.0);
            }
        }

        double rhs = 1.0;
        char sense = 'E';
        int matbeg = 0;
        CHECKED_CPX_CALL(CPXaddrows, env, lp, 0, 1, idx.size(), &rhs, &sense, &matbeg, idx.data(), coef.data(), NULL, NULL);
    }

    // Constraints: the sum of incoming arcs to j
    for (int j = 0; j < N; j++) {
        std::vector<int> idx;
        std::vector<double> coef;

        for (int i = 1; i < N; i++) {
            if (i == j) continue;
            if (map_y[i][j] >= 0) {
                idx.push_back(map_y[i][j]);
                coef.push_back(1.0);
            }
        }

        double rhs = 1.0;
        char sense = 'E';
        int matbeg = 0;
        CHECKED_CPX_CALL(CPXaddrows, env, lp, 0, 1, idx.size(), &rhs, &sense, &matbeg, idx.data(), coef.data(), NULL, NULL);
    }

    // Constraints: x_{ij} - (|N| - 1) y_{ij} <= 0
    for (int i = 0; i < N; i++) {
        for (int j = 1; j < N; j++) {
            if (map_x[i][j] >= 0 && map_y[i][j] >= 0) {  // Ensure variables exist
                std::vector<int> idx;
                std::vector<double> coef;
    
                // x_{ij} term (coefficient +1)
                idx.push_back(map_x[i][j]);
                coef.push_back(1.0);
    
                // y_{ij} term (coefficient -(|N|-1))
                idx.push_back(map_y[i][j]);
                coef.push_back(-(N - 1)); // COEFFICIENT
    
                // Define constraint: x_{ij} - (|N| - 1) y_{ij} ≤ 0
                double rhs = 0.0;
                char sense = 'L';
                int matbeg = 0;
    
                // Debugging: Print which constraints are being added
                std::cout << "Adding constraint: x_" << i << "_" << j << " ≤ " << (N - 1) << " * y_" << i << "_" << j << std::endl;
                CHECKED_CPX_CALL(CPXaddrows, env, lp, 0, 1, idx.size(), &rhs, &sense, &matbeg, 
                                 idx.data(), coef.data(), NULL, NULL);
            }
        }
    }

    // // Ensuring that exactly one incoming edge enters node 0
    // std::vector<int> idx;
    // std::vector<double> coef;

    // for (int i = 1; i < N; i++) {
    //     if (map_y[i][0] >= 0) {
    //         idx.push_back(map_y[i][0]);
    //         coef.push_back(1.0);
    //     }
    // }

    // double rhs = 1.0;
    // char sense = 'E';
    // int matbeg = 0;

    // std::cout << "Adding return-to-origin constraint (flow into node 0)" << std::endl;
    // CHECKED_CPX_CALL(CPXaddrows, env, lp, 0, 1, idx.size(), &rhs, &sense, &matbeg, idx.data(), coef.data(), NULL, NULL);

    std::cout << "Finished adding constraints." << std::endl;
}

int main (int argc, char const *argv[])
{
    std::vector<Hole> holes = readBoard("board.dat");
    std::vector<std::vector<double>> C = computeCostMatrix(holes);

    try {
        DECL_ENV(env);
        DECL_PROB(env, lp);

        setupLP(env, lp, C, holes.size()); // Pass computed cost matrix

        CHECKED_CPX_CALL(CPXwriteprob, env, lp, "debug_model.lp", NULL);
        
        CHECKED_CPX_CALL(CPXsetdblparam, env, CPX_PARAM_EPRHS, 1e-9);
        
        //optimize
        std::cout << "Starting optimization..." << std::endl;
        CHECKED_CPX_CALL(CPXmipopt, env, lp);
        std::cout << "Optimization completed." << std::endl;

        double objval;
        CHECKED_CPX_CALL(CPXgetobjval, env, lp, &objval);
        std::cout << "Objective value: " << objval << std::endl;

        // Print overall solution information on a .sol file
        CHECKED_CPX_CALL( CPXsolwrite, env, lp, "drill.sol" );

        CPXfreeprob(env, &lp);
        CPXcloseCPLEX(&env);
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

}

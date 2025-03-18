#include <cstdio>
#include <iostream>
#include <vector>
#include "cpxmacro.h"

using namespace std;

// error status and message buffer (from cpxmacro.h)
int status;
char errmsg[BUF_SIZE];

// Data
const int N = 4;

// Cost matrix (NxN, not NxA)
const double C[N][N] = { {1.0, 1.0, 1.0, 1.0},
                         {1.0, 1.0, 1.0, 1.0},
                         {1.0, 1.0, 1.0, 1.0},
                         {1.0, 1.0, 1.0, 1.0} };

const int NAME_SIZE = 512;
char name[NAME_SIZE];

/*MAP FOR X, Y VARS*/
vector<vector<int>> map_x;  // x_ij ---> map_x[i][j]
vector<vector<int>> map_y;  // y_ij ---> map_y[i][j]

void setupLP(CEnv env, Prob lp)
{
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

}

int main (int argc, char const *argv[])
{
    try
    {
        ///// init
        DECL_ENV( env );
        DECL_PROB( env, lp );
        
        ///// setup LP
        setupLP(env, lp);

        CHECKED_CPX_CALL(CPXwriteprob, env, lp, "debug_model.lp", NULL);
        
        CHECKED_CPX_CALL(CPXsetdblparam, env, CPX_PARAM_EPRHS, 1e-9);

        ///// optimize
        CHECKED_CPX_CALL( CPXmipopt, env, lp );
        
        ///// print
        // print objective function value
        double objval;
        CHECKED_CPX_CALL( CPXgetobjval, env, lp, &objval );
        std::cout << "Objval: " << objval << std::endl;
        
        // print value of decision variables
        std::vector<double> varvals; // place to store variable values
        int n = CPXgetnumcols(env, lp);    // get number of variables
        varvals.resize(n);
        int fromIdx = 0;
        int toIdx = n - 1;
        CHECKED_CPX_CALL(CPXgetx, env, lp, varvals.data(), fromIdx, toIdx);
        for (int p = 0; p < n; ++p) {
            // to skip all the zero-valued variables
            if (varvals[p] <= 1e-6 && varvals[p] >= -1e-6) continue;
            std::cout << "var in position " << p << " has optimal value " << varvals[p] << std::endl;
        }

        // Print y variables
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                if (map_y[i][j] >= 0) {
                    if (std::abs(varvals[map_y[i][j]]) < 1e-6) 
                        varvals[map_y[i][j]] = 0;
                    std::cout << "y_" << i << "_" << j << " = " << varvals[map_y[i][j]] << std::endl;
                }
            }
        }

        // Print x variables
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                if (map_x[i][j] >= 0) {
                    std::cout << "x_" << i << "_" << j << " = " << varvals[map_x[i][j]] << std::endl;
                }
            }
        }

        // Print flow
        for (int i = 0;
             i < N;
             i++) {
            for (int j = 0; j < N; j++) {
                if (map_x[i][j] >= 0) {
                    std::cout << "Flow from " << i << " to " << j << " = " << varvals[map_x[i][j]] << std::endl;
                }
            }
        }

        // Print overall solution information on a .sol file
        CHECKED_CPX_CALL( CPXsolwrite, env, lp, "drill.sol" );

        ///// free memory
        CPXfreeprob(env, &lp);
        CPXcloseCPLEX(&env);
    }
    catch(std::exception& e)
    {
        std::cout << ">>>EXCEPTION: " << e.what() << std::endl;
    }
    return 0;
}

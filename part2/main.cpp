/**
 * @file main.cpp
 * @brief 
 */


#include <stdexcept>
#include <ctime>
#include <sys/time.h>

#include "TSPSolver.h"

// error status and messagge buffer
int status;
char errmsg[255];
int tabuLength = 10;
int maxIterations = 1000;


int main (int argc, char const *argv[])
{
  try
  {
    if (argc < 2) throw std::runtime_error("usage: ./main filename.dat [--alpha=0.7 --beta=0.5 --decayFactor=0.9 --lambda=0.01 --logFile=log.txt]");

    // Default parameters
    double alpha = 0.75;
    double beta = 0.5;
    double decayFactor = 0.9;
    double lambda = 0.01;
    std::string logFileName = ""; // Default empty, will be set from argument or derived

    // parsing
    for (int i = 2; i < argc; ++i) {
      std::string arg = argv[i];
      if (arg.find("--alpha=") == 0) {
        alpha = std::stod(arg.substr(8));
      } else if (arg.find("--beta=") == 0) {
        beta = std::stod(arg.substr(7));
      } else if (arg.find("--decayFactor=") == 0) {
        decayFactor = std::stod(arg.substr(14));
      } else if (arg.find("--lambda=") == 0) {
        lambda = std::stod(arg.substr(9));
      } else if (arg.find("--logFile=") == 0) {
        logFileName = arg.substr(10);
      } else {
        std::cerr << "Warning: Unknown parameter: " << arg << std::endl;
      }
    }
    
    /// create the instance (reading data)
    TSP tspInstance;
    tspInstance.read(argv[1]);

    // If --logFile was not provided, derive it from the input filename
    if (logFileName.empty()) {
      std::string inputFile = argv[1];
      size_t dotPos = inputFile.find_last_of('.');
      if (dotPos != std::string::npos) {
          logFileName = inputFile.substr(0, dotPos) + "_log.txt";
      } else {
          logFileName = inputFile + "_log.txt";
      }
    }

    TSPSolution aSolution(tspInstance);
    
    /// initialize clocks for running time recording
    ///   two ways:
    ///   1) CPU time (t2 - t1)
    clock_t t1,t2;
    t1 = clock();
    ///   2) wall-clock time (tv2 - tv1)
    struct timeval  tv1, tv2;
    gettimeofday(&tv1, NULL);
    
    /// create solver class
    TSPSolver tspSolver(logFileName, alpha, beta, decayFactor, lambda);
    /// initial solution (random)
    tspSolver.initRnd(aSolution);
    
    /// run the neighbourhood search
    TSPSolution bestSolution(tspInstance);
    tspSolver.solve(tspInstance, aSolution, tabuLength, maxIterations, bestSolution);
    
    /// final clocks
    t2 = clock();
    gettimeofday(&tv2, NULL);
    
    std::cout << "FROM solution: "; 
    aSolution.print();
    std::cout << "(value : " << tspSolver.evaluate(aSolution,tspInstance) << ")\n";
    std::cout << "TO   solution: "; 
    bestSolution.print();
    std::cout << "(value : " << tspSolver.evaluate(bestSolution,tspInstance) << ")\n";
    std::cout << "in " << (double)(tv2.tv_sec+tv2.tv_usec*1e-6 - (tv1.tv_sec+tv1.tv_usec*1e-6)) << " seconds (user time)\n";
    std::cout << "in " << (double)(t2-t1) / CLOCKS_PER_SEC << " seconds (CPU time)\n";
    std::cout << "FINAL_VALUE: " << tspSolver.evaluate(bestSolution, tspInstance) << std::endl;
  }
  catch(std::exception& e)
  {
    std::cout << ">>>EXCEPTION: " << e.what() << std::endl;
  }
  return 0;
}

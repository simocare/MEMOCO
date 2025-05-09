/**
 * @file TSP.h
 * @brief TSP data
 *
 */

 #ifndef TSP_H
 #define TSP_H
 
 #include <stdlib.h>
 #include <stdio.h>
 #include <iostream>
 #include <fstream>
 #include <vector>
 #include <cmath>
 
 /**
  * Class that describes a TSP instance (a cost matrix, nodes are identified by integer 0 ... n-1)
  */
 class TSP
 {
 public:
   TSP() : n(0) , infinite(1e10) { }
   int n; //number of nodes
   std::vector< std::vector<double> > cost;
   double infinite; // infinite value (an upper bound on the value of any feasible solution)
 
   void read(const char* filename)
   {
       std::ifstream file(filename);
       if (!file) {
           std::cerr << "Cannot open file.\n";
           return;
       }
 
       int gridSize;
       file >> gridSize;
 
       std::vector<std::vector<int>> grid(gridSize, std::vector<int>(gridSize));
       std::vector<std::pair<int, int>> holes;
 
       for (int i = 0; i < gridSize; ++i) {
           for (int j = 0; j < gridSize; ++j) {
               int val;
               file >> val;
               grid[i][j] = val;
               if (val == 1) {
                   holes.push_back({j, i});  // (x = col, y = row)
               }
           }
       }
 
       n = holes.size();
       std::cout << "Extracted " << n << " holes from grid.\n";
 
       cost.resize(n, std::vector<double>(n, 0.0));
 
       for (int i = 0; i < n; ++i) {
           for (int j = 0; j < n; ++j) {
               if (i != j) {
                   double dx = holes[i].first - holes[j].first;
                   double dy = holes[i].second - holes[j].second;
                   cost[i][j] = std::sqrt(dx * dx + dy * dy);
               }
           }
       }
 
       // Set infinite value as upper bound
       infinite = 0;
       for (int i = 0; i < n; ++i)
           for (int j = 0; j < n; ++j)
               infinite += cost[i][j];
       infinite *= 2;
   }
 };
 
 #endif /* TSP_H */
 
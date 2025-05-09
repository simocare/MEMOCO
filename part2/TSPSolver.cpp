/**
 * @file TSPSolver.cpp
 * @brief TSP solver (neighborhood search)
 *
 */

#include "TSPSolver.h"
#include <iostream>

bool TSPSolver::solve ( const TSP& tsp , const TSPSolution& initSol , int tabulength , int maxIter , TSPSolution& bestSol )   /// TS: new param
{
  try
  {
    bool stop = false;
    int  iter = 0;

    std::ofstream log("tsp_log.txt");

    ///Tabu Search
    tabuLength = tabulength;
    tabuList.reserve(tsp.n);
    initTabuList(tsp.n);
    ///
    
    TSPSolution currSol(initSol);
    double bestValue, currValue;
    bestValue = currValue = evaluate(currSol,tsp);

    std::cout << "Initial solution: ";
    currSol.print();
    std::cout << " (value : " << currValue << ")" << std::endl;

    log << "TOUR ";
    for (int city : currSol.sequence) log << city << " ";
    log << "\nVALUE " << currValue << "\n";

    TSPMove move;
    while ( ! stop ) {
      ++iter;                                                                                             /// TS: iter not only for displaying
      				if ( tsp.n < 20 ) currSol.print();
      				std::cout << " (" << iter << "ac) value " << currValue << "\t(" << evaluate(currSol,tsp) << ")";
              log << "ITERATION " << iter << "\n";
              log << "TOUR ";
              for (int city : currSol.sequence) log << city << " ";
              log << "\n";
              log << "VALUE " << currValue << "\n";


      double aspiration = bestValue-currValue;                                                            //**// TSAC: aspired IMPROVEMENT (to improve over bestValue)
      double bestNeighValue = currValue + findBestNeighbor(tsp,currSol,iter,aspiration,move);             //**// TSAC: aspiration
      //if ( bestNeighValue < currValue ) {         /// TS: replace stopping and moving criteria
      //  bestValue = currValue = bestNeighValue;   ///
      //  currSol = apply2optMove(currSol,move);             ///
      //  stop = false;                             ///
      //} else {                                    ///
      //  stop = true;                              ///
      //}                                           ///
      
      if ( bestNeighValue >= tsp.infinite ) {       /// TS: stop because all neighbours are tabu
        std::cout << "\tmove: NO legal neighbour" << std::endl;   ///
        log << "NO legal neighbour\n";
        stop = true;                                ///
        continue;                                   ///
      }                                             ///
      
      std::cout << "\tmove: " << move.from << " , " << move.to;       // NEXT MOVE that we are going to apply after the current iteration
      log << "MOVE " << move.from << " , " << move.to << "\n";

      if ( currValue < bestValue - 0.01 ) {
        log << "ASPIRATION_ACCEPTED\n";
      }
      
			updateTabuList(currSol.sequence[move.from],currSol.sequence[move.to],iter);	/// TS: insert move info into tabu list
			      
			currSol = apply2optMove(currSol,move);                                                                       /// TS: always the best move
      currValue = bestNeighValue;                                                                         /// 
      if ( currValue < bestValue - 0.01 ) {					/// TS: update incumbent (if better -with tolerance- solution found)
        bestValue = currValue;                                                                            ///
        bestSol = currSol;                                                                                ///
        			std::cout << "\t***";                                                                       ///
      }                                                                                                   /// 
      
      if ( iter > maxIter ) {                                                                             /// TS: new stopping criteria
        stop = true;                                                                                      ///
      }                                                                                                   ///
      				std::cout << std::endl;
    }
    //bestSol = currSol;                            /// TS: not always the neighbor improves over 
                                                                                                          ///     the best available (incumbent) solution 
    log << "FINAL_SOLUTION\n";
    for (int city : bestSol.sequence) log << city << " ";
    log << "\n";
    log << "FINAL_VALUE " << bestValue << "\n";
    log.close();                                                                                         
  }
  catch(std::exception& e)
  {
    std::cout << ">>>EXCEPTION: " << e.what() << std::endl;
    return false;
  }
  
  return true;
}

TSPSolution& TSPSolver::apply2optMove ( TSPSolution& tspSol , const TSPMove& move ) 
{
  TSPSolution tmpSol(tspSol);
  for ( int i = move.from ; i <= move.to ; ++i ) {
    tspSol.sequence[i] = tmpSol.sequence[move.to-(i-move.from)];
  }
  return tspSol;
}


double TSPSolver::findBestNeighbor ( const TSP& tsp , const TSPSolution& currSol , int currIter , double aspiration , TSPMove& move )     //**// TSAC: use aspiration
/* Determine the NON-TABU *move* yielding the best 2-opt neigbor solution 
 * Aspiration criteria: 'neighCostVariation' better than 'aspiration' (notice that 'aspiration'
 * has been set such that if 'neighCostVariation' is better than 'aspiration' than we have a
 * new incumbent solution)
 */
{
  double bestCostVariation = tsp.infinite;

  // intial and final position are fixed (initial/final node remains 0)
  for ( uint a = 1 ; a < currSol.sequence.size() - 2 ; a++ ) {
    int h = currSol.sequence[a-1];
    int i = currSol.sequence[a];
    for ( uint b = a + 1 ; b < currSol.sequence.size() - 1 ; b++ ) {
      int j = currSol.sequence[b];
      int l = currSol.sequence[b+1];
			//**// TSAC: to be checked after... if (isTabu(i,j,currIter)) continue;						/// TS: tabu check (just one among many ways of doing it...) 
      double neighCostVariation = - tsp.cost[h][i] - tsp.cost[j][l]
                                  + tsp.cost[h][j] + tsp.cost[i][l] ;
      if ( isTabu(i,j,currIter) && !(neighCostVariation < aspiration-0.01) ) {
        continue;             //**// TSAC: check if tabu and not aspiration criteria
			}
      if ( neighCostVariation < bestCostVariation ) {
        bestCostVariation = neighCostVariation;
        move.from = a;
        move.to = b;
      }
			//if ( bestCostVariation < 0 ) return bestCostVariation; //*****// First Improvement variant (in this case a better way of determining the order in which neighbor are explored should be implemented)
    }
  }
  return bestCostVariation;
}

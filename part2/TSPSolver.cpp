/**
 * @file TSPSolver.cpp
 * @brief TSP solver (neighborhood search)
 *
 */

#include "TSPSolver.h"
#include <iostream>

bool TSPSolver::solve ( const TSP& tsp , const TSPSolution& initSol , int tabulength , int maxIter , TSPSolution& bestSol)
{
  try
  {
    bool stop = false;
    int  iter = 0;

    ///Tabu Search
    tabuLength = tabulength;
    tabuList.reserve(tsp.n);
    initTabuList(tsp.n);
    ///
    int noImproveThreshold = static_cast<int>(alpha * tsp.n);
    int tenureAdaptThreshold = static_cast<int>(beta * noImproveThreshold);
    int shakeThreshold = noImproveThreshold;

    if (shakeThreshold <= tenureAdaptThreshold) {
      throw std::logic_error("shakeThreshold must be > tenureAdaptThreshold");
    }
    
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

    const double epsilon = 0.01;
    static int iterationsSinceImprovement = 0;
    static bool tenureIncreased = false;
    static int oldTenure = 0;

    while ( ! stop ) {
      ++iter;                                                                                             /// TS: iter not only for displaying
      if ( tsp.n < 20 ) currSol.print();
      std::cout << " (" << iter << "ac) value " << currValue << "\t(" << evaluate(currSol,tsp) << ")";
      log << "ITERATION " << iter << "\n";
      log << "TOUR ";
      for (int city : currSol.sequence) log << city << " ";
      log << "\n";
      log << "VALUE -> " << currValue << "\n";


      double aspiration = bestValue-currValue;                                                          //**// TSAC: aspired IMPROVEMENT (to improve over bestValue)
      double bestCostVariation = findBestNeighbor(tsp,currSol,iter,aspiration,move);
      double printableVariation = std::abs(bestCostVariation) < 1e-10 ? 0.0 : bestCostVariation;
      log << "BEST_COST_VARIATION " << printableVariation << "\n";
      double bestNeighValue = currValue + bestCostVariation;                                            //**// TSAC: aspiration
      //if ( bestNeighValue < currValue ) {         /// TS: replace stopping and moving criteria; SIMONE: too simple (it would stop too soon) -> do not use
      //  bestValue = currValue = bestNeighValue;   ///
      //  currSol = apply2optMove(currSol,move);    ///
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
			      
			currSol = apply2optMove(currSol,move);                                      /// TS: always the best move
      currValue = bestNeighValue;
      oldTenure = tabuLength;

      log << "currValue " << currValue << "bestValue " << bestValue << "\n";
      if ( currValue < bestValue - epsilon ) {					/// TS: update incumbent (if better -with tolerance- solution found)
        bestValue = currValue;
        bestSol = currSol;
        std::cout << "\t***";
        iterationsSinceImprovement = 0;
        tenureIncreased = false;

        // --- INTENSIFICATION: reduce tenure --
        tabuLength = std::max(minTenure, tabuLength / 2);
        log << "\t*** (intensification, tenure: " << oldTenure << " -> " << tabuLength << ")\n";

      } else {
        iterationsSinceImprovement++;

        // --- DIVERSIFICATION: increase tenure if no improvement for a while ---
        log << "\t NO IMPROVEMENT; Iteration since improvement=" << iterationsSinceImprovement << " tenure=" << tabuLength << " tenureAdaptThreshold=" << tenureAdaptThreshold << " shakeThreshold=" << shakeThreshold << "\n";
        if (iterationsSinceImprovement >= tenureAdaptThreshold && !tenureIncreased) {
          tabuLength = std::min(maxTenure, tabuLength * 2);
          tenureIncreased = true;
          log << "\t(diversification, tenure: " << oldTenure << " -> " << tabuLength << ")\n";
        }

        if (iterationsSinceImprovement >= shakeThreshold) {
        // --- DOUBLE BRIDGE MOVE or REINITIALIZATION - SHAKING ---
          currSol = applyDoubleBridgeMove(currSol);
          currValue = evaluate(currSol, tsp);
          iterationsSinceImprovement = 0; // Reset: gives time to explore new area
          tenureIncreased = false;

          log << "\t shakeThreshold " << shakeThreshold << "\n";
          log << "\t(shaking applied: double-bridge move)\n";
          std::cout << "\t Shaking: double-bridge move applied" << std::endl;
        }
      }
      
      if ( iter > maxIter ) {                       /// TS: new stopping criteria
        stop = true;                                ///
      }                                             ///
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
  //logLine("entering\n");
  double bestCostVariation = tsp.infinite; // the change in total tour cost if we apply a 2-opt move

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
        logLine("tabu move\n");
			}
      //log << "-> inside: " << bestCostVariation << " " << neighCostVariation << "\n";
      if ( neighCostVariation < bestCostVariation ) {
        bestCostVariation = neighCostVariation;
        move.from = a;
        move.to = b;
      }
      // if it stays = tsp.infinite, it means that the move is not improving the tour

      //*****// First Improvement variant
			//if ( bestCostVariation < 0 ) return bestCostVariation;
      // If bestCostVariation < 0, that means the move improves the tour (because it's reducing the total cost).
      //So as soon as you find a move that improves the tour, just stop searching and return it immediately.
    }
  }
  return bestCostVariation;
}

TSPSolution TSPSolver::applyDoubleBridgeMove(const TSPSolution& sol) {
    TSPSolution newSol(sol);
    int n = newSol.sequence.size();

    // Ensure there are enough cities to perform the move
    if (n < 8) return newSol;

    // Select four break points ensuring they are in order and not too close
    int pos1 = 1 + rand() % (n / 4);
    int pos2 = pos1 + 1 + rand() % (n / 4);
    int pos3 = pos2 + 1 + rand() % (n / 4);
    int pos4 = pos3 + 1 + rand() % (n / 4);

    // Create segments
    std::vector<int> segment1(newSol.sequence.begin(), newSol.sequence.begin() + pos1);
    std::vector<int> segment2(newSol.sequence.begin() + pos1, newSol.sequence.begin() + pos2);
    std::vector<int> segment3(newSol.sequence.begin() + pos2, newSol.sequence.begin() + pos3);
    std::vector<int> segment4(newSol.sequence.begin() + pos3, newSol.sequence.begin() + pos4);
    std::vector<int> segment5(newSol.sequence.begin() + pos4, newSol.sequence.end());

    // Reconnect segments in new order: segment1 + segment3 + segment2 + segment4 + segment5
    newSol.sequence.clear();
    newSol.sequence.insert(newSol.sequence.end(), segment1.begin(), segment1.end()); // Appending the contents of segment1 to the end of newSol.sequence
    newSol.sequence.insert(newSol.sequence.end(), segment3.begin(), segment3.end());
    newSol.sequence.insert(newSol.sequence.end(), segment2.begin(), segment2.end());
    newSol.sequence.insert(newSol.sequence.end(), segment4.begin(), segment4.end());
    newSol.sequence.insert(newSol.sequence.end(), segment5.begin(), segment5.end());

    return newSol;
}
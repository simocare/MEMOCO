/**
 * @file TSPSolver.h
 * @brief TSP solver (neighborhood search)
 *
 */

#ifndef TSPSOLVER_H
#define TSPSOLVER_H

#include <vector>

#include "TSPSolution.h"

/**
 * Class representing substring reversal move
 */
typedef struct move {
  int      from;
  int      to;
} TSPMove;

/**
 * Class that solves a TSP problem by neighbourdood search and 2-opt moves
 */
class TSPSolver
{
public:

  TSPSolver ( ) { }

  double evaluate ( const TSPSolution& sol , const TSP& tsp ) const {
    double total = 0.0;
    for ( uint i = 0 ; i < sol.sequence.size() - 1 ; ++i ) {
      int from = sol.sequence[i]  ;
      int to   = sol.sequence[i+1];
      total += tsp.cost[from][to];
    }

    // return to initial node
    total += tsp.cost[sol.sequence.back()][sol.sequence.front()];

    return total;
  }

  bool initRnd ( TSPSolution& sol ) {
    srand(time(NULL));
    for ( uint i = 1 ; i < sol.sequence.size() ; ++i ) {
      // intial and final position are fixed (initial/final node remains 0)
      int idx1 = rand() % (sol.sequence.size()-2) + 1;
      int idx2 = rand() % (sol.sequence.size()-2) + 1;
      int tmp = sol.sequence[idx1];
      sol.sequence[idx1] = sol.sequence[idx2];
      sol.sequence[idx2] = tmp;
    }
    std::cout << "### "; sol.print(); std::cout << " ###" << std::endl;
    return true;
  }

  bool solve ( const TSP& tsp , const TSPSolution& initSol , int tabulength , int maxIter , TSPSolution& bestSol, const std::string& logFileName = "tsp_log.txt"); /// TS: new parameters

  protected:
  double    findBestNeighbor ( const TSP& tsp , const TSPSolution& currSol , int currIter , double aspiration , TSPMove& move );	//**// TSAC: use aspiration!
  TSPSolution&  apply2optMove        ( TSPSolution& tspSol , const TSPMove& move );
  
  ///Tabu search (tabu list stores, for each node, when (last iteration) a move involving that node have been chosen)
  ///  a neighbor is tabu if the generating move involves two nodes that have been chosen in the last 'tabulength' moves
  ///  that is, currentIteration - LastTimeInvolved <= tabuLength
  int               tabuLength;
  std::vector<int>  tabuList;
  void  initTabuList ( int n ) {
    for ( int i = 0 ; i < n ; ++i ) {
      tabuList.push_back(-tabuLength-1);
          // at iterarion 0, no neighbor is tabu --> iteration(= 0) - tabulistInit > tabulength --> tabulistInit < tabuLength + 0
    }
  }
	void updateTabuList( int nodeFrom, int nodeTo , int iter) {
      tabuList[nodeFrom] = iter;
      tabuList[nodeTo]   = iter;
	}
	bool isTabu( int nodeFrom, int nodeTo , int iter ) {
		return ( (iter - tabuList[nodeFrom] <= tabuLength) && (iter - tabuList[nodeTo] <= tabuLength) );
  }
///
};

#endif /* TSPSOLVER_H */

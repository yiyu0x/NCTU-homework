#include <stdio.h>
#include "page_rank.h"

#include <stdlib.h>
#include <cmath>
#include <omp.h>
#include <utility>

#include "../common/CycleTimer.h"
#include "../common/graph.h"

// pageRank --
//
// g:           graph to process (see common/graph.h)
// solution:    array of per-vertex vertex scores (length of array is num_nodes(g))
// damping:     page-rank algorithm's damping parameter
// convergence: page-rank algorithm's convergence threshold
//
void pageRank(Graph g, double *solution, double damping, double convergence)
{
  // print_graph(g);
  // initialize vertex weights to uniform probability. Double
  // precision scores are used to avoid underflow for large graphs
  int numNodes = num_nodes(g);
  double equal_prob = 1.0 / numNodes;
  #pragma omp parallel for
  for (int i = 0; i < numNodes; ++i) {
    solution[i] = equal_prob;
  }
  // For PP students
  double* score_old = solution;
  double* score_new = new double[numNodes];
  bool converged = false;
  double global_diff = 0;
  double tmp_sum = 0;
  #pragma omp parallel for
  for (int vi = 0; vi < numNodes; ++vi) {
    score_old[vi] = equal_prob;
  }

  while (!converged) {
    
    global_diff = 0.0;
    tmp_sum = 0;
    #pragma omp parallel for reduction(+: tmp_sum) schedule(static)
    for (int i = 0; i < numNodes; i++) {
      score_new[i] = 0.0;
      const Vertex* in_begin = incoming_begin(g, i);
      const Vertex* in_end = incoming_end(g, i);
      // #pragma omp parallel for
      for (const Vertex* v = in_begin; v < in_end; v++) {
        score_new[i] += score_old[*v] / outgoing_size(g, *v);
      }
      score_new[i] = damping * score_new[i] + ((1.0 - damping) / numNodes);
      if (outgoing_size(g, i) == 0) {
        tmp_sum += score_old[i];
      }
    }
    #pragma omp parallel for reduction(+: global_diff)
    for (int i = 0; i < numNodes; i++) { 
      score_new[i] += damping * tmp_sum / numNodes;
      global_diff += abs(score_new[i] - score_old[i]);
    }
    converged = (global_diff < convergence);
    std::copy(score_new, score_new + numNodes, score_old);
  }
  delete[] score_new;
  /*
     For PP students: Implement the page rank algorithm here.  You
     are expected to parallelize the algorithm using openMP.  Your
     solution may need to allocate (and free) temporary arrays.

     Basic page rank pseudocode is provided below to get you started:

     // initialization: see example code above
     score_old[vi] = 1/numNodes;

     while (!converged) {

       // compute score_new[vi] for all nodes vi:
       score_new[vi] = sum over all nodes vj reachable from incoming edges
                          { score_old[vj] / number of edges leaving vj  }
       score_new[vi] = (damping * score_new[vi]) + (1.0-damping) / numNodes;

       score_new[vi] += sum over all nodes v in graph with no outgoing edges
                          { damping * score_old[v] / numNodes }

       // compute how much per-node scores have changed
       // quit once algorithm has converged

       global_diff = sum over all nodes vi { abs(score_new[vi] - score_old[vi]) };
       converged = (global_diff < convergence)
     }

   */
}

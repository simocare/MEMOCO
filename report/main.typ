#import "lib.typ": *
#import "@preview/mitex:0.2.5": *
#import "@preview/equate:0.3.2": equate

// CHANGE THESE
#show: report.with(
  title: "Tabu Search and CPLEX Approaches to the Traveling Salesman Problem for PCB Drilling Optimization",
  authors: (
    "Simone Caregnato",
  ),
  //group_name: "Group 14",
  //course_code: "IKT123-G",
  email_id: "simone.caregnato@studenti.unipd.it - 2154604",
  course_name: "Methods and Models for Combinatorial Optimization",
  date: "A.Y. 2024/2025",
  lang: "en", // use "no" for norwegian
)

// neat code
#import "@preview/codly:1.3.0": *
#import "@preview/codly-languages:0.1.1": *
#show: codly-init.with()
#show: equate.with(breakable: true, sub-numbering: true)
#set math.equation(numbering: "(1.1)")

= Introduction
// This report investigates the application of both exact and heuristic methods to the Traveling Salesman Problem (TSP), modeled after an industrial scenario involving printed circuit board (PCB) drilling. The goal is to find the shortest path for a drill to visit all holes on a grid-like board and return to the starting point. Two solution strategies are explored: an exact solver based on the CPLEX API and a heuristic approach using a highly customizable Tabu Search (TS) algorithm

This project addresses an optimization problem arising in the manufacturing of printed circuit boards (PCBs). Each board contains a set of holes that must be drilled, and the drill head must move across the board to reach all positions before returning to the starting point. Since the layout is repeated over many boards, minimizing the travel path can significantly reduce production time and increase efficiency.

To solve this, we implement and compare two approaches:

- an *exact method*, based on a mathematical formulation solved with the *CPLEX* optimization library;

- a *heuristic method*, using a *Tabu Search* algorithm designed to explore the solution space efficiently and return high-quality results within limited time constraints.

== Problem description
The task can be reduced to a classic Travelling Salesman Problem (TSP), where each hole on the board corresponds to a node, and the drill must visit every node exactly once and return to the starting point. The goal is to find the tour (a permutation of nodes) that minimizes the total travel distance, assuming the drilling time at each hole is constant and can thus be ignored in the optimization.

Formally, the problem can be represented on a complete weighted graph $G = (N, A)$, where:

- *Sets*:

 $N$: set of nodes representing the holes
 
 $A$: set of arcs $(i, j)$ with $i, j in N$, representing valid transitions between holes

- *Parameters*:

  $c_(i j)$: Euclidean distance between node $i$ and node $j$
  
  Node $0$: arbitrarily selected starting node

- *Decision Variables*:

  $x_(i j) \in bb(R)^+$: flow of a fictitious commodity from node $i$ to node $j$
  
  $y_(i j) \in {0,1}$: equals 1 if arc $(i,j)$ is used in the tour, 0 otherwise

- *MILP Formulation*:

$
min sum_((i, j) in A) c_(i j) y_(i j)
$

subject to:

//TODO : CONTROLLARE LE FORMULE

// #mitex(`
// \begin{align}
// \sum_{i : (i,k) \in A} x_{ik} - \sum_{j : (k,j) \in A,\, j \neq 0} x_{kj} &= 1 
//   &&\quad \forall k \in N \setminus \{0\} \tag{2} \\
// \sum_{j : (i,j) \in A} y_{ij} &= 1 
//   &&\quad \forall i \in N \tag{3} \\
// \sum_{i : (i,j) \in A} y_{ij} &= 1 
//   &&\quad \forall j \in N \tag{4} \\
// x_{ij} &\leq (|N| - 1) y_{ij} 
//   &&\quad \forall (i,j) \in A,\, j \neq 0 \tag{5} \\
// x_{ij} &\in \mathbb{R}^+ 
//   &&\quad \forall (i,j) \in A,\, j \neq 0 \tag{6} \\
// y_{ij} &\in \{0, 1\} 
//   &&\quad \forall (i,j) \in A \tag{7}
// \end{align}
// `)<constraints>

$
sum_(i :(i, k) in A) x_(i k) - sum_(j :(k, j) in A, thin j != 0) x_(k j) &= 1
& & quad forall k in N without {0} #<con1> \
sum_(j :(i, j) in A) y_(i j) &= 1
& & quad forall i in N #<con2> \
sum_(i :(i, j) in A) y_(i j) &= 1
& & quad forall j in N #<con3> \
x_(i j) & <= (|N| - 1) y_(i j)
& & quad forall(i, j) in A, thin j != 0 #<con4> \
x_(i j) & in RR^+
& & quad forall(i, j) in A, thin j != 0 #<con5> \
y_(i j) & in {0, 1}
& & quad forall(i, j) in A #<con6>
$

This compact flow-based formulation ensures subtour elimination and enforces valid transitions in the tour while minimizing the total travel cost.

== Assumption
To ensure comparability across different test cases and maintain a clear problem definition, several assumptions were made during the modeling and implementation phases.

The drill is assumed to move freely in all directions, including diagonally, with constant speed. As a consequence, the cost associated with moving from one hole to another is defined solely by the Euclidean distance between them. The time required to perform the actual drilling is constant across all holes and therefore excluded from the optimization objective.

Boards are generated randomly based on two parameters: the board size and a density value $d in [0,1]$. The density controls the number of holes on the board. Hole positions are then sampled randomly across the grid, ensuring no duplicates.

To avoid trivial instances and reduce solving times for the exact model, only boards with at least 3 holes and at most 60% filled cells are considered. The generated files follow a naming convention that encodes the size, density, and repetition index of the instance, enabling automated retrieval of tuned parameters and results.

== Input representation
All instances are based on square boards, represented as $s times s$ grids. Each board is stored in a `.dat` file, where the first line contains the board size $s$, followed by a matrix of 0s and 1s. A value of `1` indicates the presence of a hole, while `0` denotes an empty cell.

== Instance generation
The generation of input instances is handled via a C++ routine that produces random square drill boards of variable size and density. Each instance corresponds to a grid of size $s times s$, where a set number of holes is randomly placed according to a specified density parameter.

The number of holes $n$ is determined as $n = floor.l d dot.c s^2 floor.r$, and these are sampled without repetition using the C++ <set> container to avoid duplicates. Hole positions are uniformly distributed across the grid.

The instance generator is integrated directly into the parameter tuning and benchmarking pipeline, which automatically generates multiple boards per configuration (size, density, repetition index). The naming of the .dat files reflects these parameters, facilitating consistent referencing across experiments.

= Part I - CPLEX Solver

== Implementation details
The exact approach is implemented in C++ using the CPLEX Callable Library. The main routine is responsible for loading the instance, computing the distance matrix, building the MILP model, invoking the solver, and exporting the results.

After parsing the `.dat` file and extracting hole positions (stored in a `std::vector<Hole>` where `Hole` is a simple struct representing the coordinates of each hole), a cost matrix is computed using pairwise Euclidean distances. Each node is uniquely identified by its index in the hole list.

The model is constructed inside the `setupLP` function, which receives the CPLEX environment, problem pointer, distance matrix, and number of holes. Two sets of variables are declared:

- binary variables $y_(i j)$, one for each arc $(i, j)$, are used to represent the selection of edges in the final tour;
- continuous variables $x_(i j)$ are used to implement the subtour elimination constraints based on a flow model.

For each pair $(i, j)$, the corresponding variables are added using `CPXnewcols`, and their indices are stored in the lookup tables `map_x[i][j]` and `map_y[i][j]`.

Constraints are then added to the model in three groups:

- *Flow conservation* @con1: For each internal node $k$, the incoming and outgoing flow must sum to 1, enforcing connectivity and eliminating subtours.
- *Degree constraints* (@con2 & @con3): For each node, exactly one incoming and one outgoing arc must be selected.
- *Coupling constraints* (@con4): Ensure that flow can only travel on arcs included in the solution, by bounding x with y.

Each group of constraints is encoded by creating sparse row representations and passed to `CPXaddrows` in batch. Names and bounds for variables and constraints are generated using standard string formatting. The macros provided in `cpxmacro.h` handle environment and error checks.

Once the model is complete, `CPXmipopt` is called to solve the instance. The solution cost is printed to the terminal and also saved to a `.sol` file using `CPXsolwrite`. Timing is tracked via `chrono` to measure solver performance.
  
= Part II - Tabu Search

== Data structure
vector of holes (cities) indices

== Neighborhood and Move Evaluation

== Tabu list representation

== Aspiration and stopping criteria
Simple: 1 accept if bestValue, 2 stop if maxIter (we keep this simple to focus on the next aspects of this method)

== Intensification and diversification

=== Adaptive Tenure - Reactive search
also initialized based on size/density of the board

=== Intensification
==== Randomized restart from elite solutions (or elite memory)

=== Diversification
==== Shaking mechanism
==== Frequency-based penalties

== Results
=== Parameters Tuning
=== Benchmarking framework
to compare results with cplex (using best params)

= Conclusion

= Setup and execution istructions
TODO

= Examples
== Citation
This is something stated from a source @example-source.

== Tables
Here's a table:
#figure(
  caption: [Table of numbers],
  table(
    columns: (auto, auto),
    inset: 10pt,
    align: horizon,
    table.header(
      [*Letters*], [*Number*], 
    ),
    [Five], [5],
    [Eight], [8],
  ) 
)

== Code blocks
Here's a rust code block:
#figure(
  caption: [Epic code],
  ```rs
  fn main() {
      let name = "buddy";
      let greeting = format!("Hello, {}!", name);
      println!("{}", greeting);
  }
  ```
)

== Math
Here's some math:
$ integral_0^infinity e^(-x^2) dif x = sqrt(pi)/2 $
And some more: $sigma / theta dot i$.
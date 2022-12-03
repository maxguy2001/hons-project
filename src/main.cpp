#include "../lib/core/types.hpp"
#include "../lib/logical_solver/presolve.hpp"
#include "../lib/utils/reader.hpp"
#include "../lib/utils/primal_reader.hpp"
#include <vector>
#include <iostream>
#include <fstream>
#include <string>

int main(){
  std::string test_problem = "/Users/pepe/hons-project/problems/presolve_test_problem.txt";
  std::string all_test_problems = "/Users/pepe/hons-project/problems/feasibility_testcases.txt";
  utils::Reader reader;

  // Start filestream to pass to reader.
  std::fstream problems_filestream;
  problems_filestream.open(test_problem, std::ios::in);

  // std::vector<std::vector<int>> problem_matrix;
  // std::vector<int> upper_bounds;
  // std::vector<int> lower_bounds;
  reader.readNextProblem(problems_filestream);

  logical_solver::Presolve presolve(
    reader.problem_matrix_, 
    reader.lower_bounds_, reader.upper_bounds_,
    reader.num_inequalities_, reader.num_equalities_
  );
  problems_filestream.close();

  //presolve.printLP();
  presolve.applyPresolve();
  presolve.applyPostsolve();
  presolve.printFeasibleSolution();
  //presolve.printLP();

  return 0;
}
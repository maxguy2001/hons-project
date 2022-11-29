#include "../lib/core/types.hpp"
#include "../lib/logical_solver/presolve.hpp"
#include "../lib/utils/reader.hpp"
#include "../lib/utils/primal_reader.hpp"
#include <vector>
#include <iostream>
#include <fstream>
#include <string>

int main(){
  std::string test_problems = "/Users/pepe/hons-project/problems/feasibility_testcases.txt";
  utils::Reader reader;

  // Start filestream to pass to reader.
  std::fstream problems_filestream;
  problems_filestream.open(test_problems, std::ios::in);

  std::vector<std::vector<int>> problem_matrix;
  std::vector<int> upper_bounds;
  std::vector<int> lower_bounds;

  for (int i = 0; i < 1; i++){
    std::cout << i << std::endl;
    reader.readNextProblem(problems_filestream);
    problem_matrix = reader.problem_matrix_;
    upper_bounds = reader.upper_bounds_;
    lower_bounds = reader.lower_bounds_;
    
  }

  logical_solver::Presolve presolve(problem_matrix, lower_bounds, upper_bounds);
  presolve.printLP();

  return 0;
}
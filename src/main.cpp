#include "../lib/core/types.hpp"
#include "../lib/logical_solver/presolve.hpp"
#include "../lib/utils/reader.hpp"
#include "../lib/utils/primal_reader.hpp"
#include <vector>
#include <iostream>
#include <fstream>
#include <string>

void testOnTestProblem() {
  std::string test_problem = "/Users/pepe/hons-project/problems/presolve_test_problem.txt";
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
}

void testOnSingleProblem(int problem_number) {
  std::string all_test_problems = "/Users/pepe/hons-project/problems/feasibility_testcases.txt";
  utils::Reader reader;
  reader.readProblem(all_test_problems, problem_number);

  logical_solver::Presolve presolve(
    reader.problem_matrix_, 
    reader.lower_bounds_, reader.upper_bounds_,
    reader.num_inequalities_, reader.num_equalities_
  );
  presolve.printLP();
  presolve.applyPresolve();
  if (presolve.reduced_to_empty) {
    printf("Reduced to empty\n");
  }
  // presolve.applyPostsolve();
  // presolve.printFeasibleSolution();
}

void testOnMultipleProblems(int problems_count) {
  std::string all_test_problems = "/Users/pepe/hons-project/problems/feasibility_testcases.txt";
  utils::Reader reader;

  // Start filestream to pass to reader.
  std::fstream problems_filestream;
  problems_filestream.open(all_test_problems, std::ios::in);

  int reduced_to_empty_count = 0;
  for (int n = 0; n < problems_count; n++) {
    printf("Solving problem %d\n", n);

    // Reading problem.
    reader.readNextProblem(problems_filestream);

    if (reader.problem_matrix_.size() > 0) {
      logical_solver::Presolve presolve(
        reader.problem_matrix_, 
        reader.lower_bounds_, reader.upper_bounds_,
        reader.num_inequalities_, reader.num_equalities_
      );
      presolve.applyPresolve();

      if (presolve.reduced_to_empty) {
        std::cout << "REDUCED TO EMPTY\n" << std::endl;
        reduced_to_empty_count += 1;
        presolve.applyPostsolve();
      }
    }
  }
  printf("%d problems were reduced to empty\n", reduced_to_empty_count);
}

int main(){
  // testOnTestProblem();
  // testOnSingleProblem(2203);
  const int all_test_cases_count = 150218;
  testOnMultipleProblems(all_test_cases_count);


  return 0;
}
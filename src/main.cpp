#include "../lib/core/types.hpp"
#include "../lib/logical_solver/presolve.hpp"
#include "../lib/utils/reader.hpp"
#include "../lib/utils/primal_reader.hpp"
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <chrono>

void testOnTestProblem() {
  std::string test_problem = "/Users/pepe/hons-project/problems/presolve_test_problem2.txt";
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
    reader.num_inequalities_, reader.num_equalities_,
    false
  );
  problems_filestream.close();

  presolve.printLP();
  presolve.applyPresolve();
  presolve.applyPostsolve();
  if (presolve.reduced_to_empty) {
    printf("Reduced to empty\n");
  }
  if (!presolve.infeasible){
    presolve.printFeasibleSolution();
  }
  else {
    std::cout << "Infeasible" << std::endl;
  }
  // presolve.printImpliedBounds();
}

void testOnSingleProblem(int problem_number) {
  std::string all_test_problems = "/Users/pepe/hons-project/problems/feasibility_testcases.txt";
  utils::Reader reader;
  reader.readProblem(all_test_problems, problem_number);

  logical_solver::Presolve presolve(
    reader.problem_matrix_, 
    reader.lower_bounds_, reader.upper_bounds_,
    reader.num_inequalities_, reader.num_equalities_,
    false
  );

  presolve.printLP();
  presolve.applyPresolve();
  if (presolve.reduced_to_empty) {
    printf("Reduced to empty\n");
  }
  presolve.applyPostsolve();
  presolve.printFeasibleSolution();
}

void testOnMultipleProblems(int problems_count) {
  std::string all_test_problems = "/Users/pepe/hons-project/problems/feasibility_testcases.txt";
  utils::Reader reader;

  // Start filestream to pass to reader.
  std::fstream problems_filestream;
  problems_filestream.open(all_test_problems, std::ios::in);

  int reduced_to_empty_count = 0;
  int infeasible_count = 0;
  int infeasible_by_parallel_rows_count = 0;
  int unsatisfied_constraints = 0;

  for (int n = 0; n < problems_count; n++) {
    // printf("Solving problem %d\n", n);

    // Reading problem.
    reader.readNextProblem(problems_filestream);

    if (reader.problem_matrix_.size() > 0) {
      logical_solver::Presolve presolve(
        reader.problem_matrix_, 
        reader.lower_bounds_, reader.upper_bounds_,
        reader.num_inequalities_, reader.num_equalities_,
        false
      );
      presolve.applyPresolve();
      presolve.applyPostsolve();
      if (presolve.unsatisfied_constraints) {
        unsatisfied_constraints += 1;
        presolve.printLP();
        presolve.printFeasibleSolution();
        std::cout<<n<<std::endl;
        break;
      }
      else if (presolve.infeasible) {
        infeasible_count += 1;
      }
      else if (presolve.reduced_to_empty) {
        // std::cout << "REDUCED TO EMPTY\n" << std::endl;
        reduced_to_empty_count += 1;
        // presolve.applyPostsolve();
      }
    }
  }
  std::cout<<""<<std::endl;
  printf("%d problems were reduced to empty.\n", reduced_to_empty_count);
  printf("%d problems were not feasible \n", infeasible_count);
  printf("%d problems led to unsatisfied constraints\n", unsatisfied_constraints);

}

int main(){
  testOnTestProblem();
  // testOnSingleProblem(755);
  // const int all_test_cases_count = 150218;
  // auto start = std::chrono::high_resolution_clock::now();
  // testOnMultipleProblems(all_test_cases_count);
  // auto elapsed = std::chrono::high_resolution_clock::now() - start;
  // long long seconds = std::chrono::duration_cast<std::chrono::seconds>(elapsed).count();
  // printf("Time taken (seconds): %d \n", int(seconds));

  return 0;
}
#include "../lib/primal_simplex/main.hpp"
#include "../lib/core/consts.hpp"
#include "../lib/revised_primal_solver/simplex.hpp"
#include "../lib/utils/modified_primal_reader.hpp"
#include "../lib/utils/primal_reader.hpp"
#include "../lib/utils/reformatter.hpp"
#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

int main() {

  const std::string pp = "/home/maxguy/projects/hons/hons-project/problems/"
                         "feasibility_testcases.txt";
  std::fstream filestream;
  filestream.open(pp, std::ios::in);

  // instantiate everything
  utils::ModifiedPrimalReader reader_(filestream);
  revised_primal_simplex::RevisedPrimalSimplex solver_;
  utils::Reformatter rf_;

  int num_to_solve = 2000;
  int num_failed = 0;

  // time the run
  std::uint64_t start_time =
      std::chrono::duration_cast<std::chrono::nanoseconds>(
          std::chrono::high_resolution_clock::now().time_since_epoch())
          .count();

  for (std::size_t i = 0; i < num_to_solve; ++i) {
    auto problem = reader_.getNextProblem();
    core::FormattedProblem rf_prob = rf_.reformatProblem(*problem);
    solver_.setProblem(rf_prob.problem_matrix);
    solver_.setBasis(rf_prob.basic_variables);
    auto solution_row = solver_.solveProblem();

    if (!solution_row) {
      ++num_failed;
    }
  }

  std::uint64_t end_time =
      std::chrono::duration_cast<std::chrono::nanoseconds>(
          std::chrono::high_resolution_clock::now().time_since_epoch())
          .count();

  std::uint64_t time_taken_nanos = end_time - start_time;
  double time_taken_secs =
      static_cast<double>(time_taken_nanos) / 1'000'000'000;

  std::cout << "Number of problems solved: " << num_to_solve << std::endl;
  std::cout << "Number of falures: " << num_failed << std::endl;
  std::cout << "Time taken: " << time_taken_secs << " seconds" << std::endl;

  return 0;
}

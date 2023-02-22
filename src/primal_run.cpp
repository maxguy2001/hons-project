#include "primal_run.hpp"

int run_primal() {

  const std::string pp = "/home/maxguy/projects/hons/hons-project/problems/"
                         "feasibility_testcases.txt";
  std::fstream filestream;
  filestream.open(pp, std::ios::in);

  // instantiate everything
  utils::ModifiedPrimalReader reader_(filestream);
  revised_primal_simplex::RevisedPrimalSimplex solver_;
  utils::Reformatter rf_;

  // problem 2019 is first basis fault!
  int num_to_solve = 10'000;
  int num_failed = 0;
  int num_empty = 0;

  // time the run
  std::uint64_t start_time =
      std::chrono::duration_cast<std::chrono::nanoseconds>(
          std::chrono::high_resolution_clock::now().time_since_epoch())
          .count();

  for (std::size_t i = 0; i < num_to_solve; ++i) {
    auto problem = reader_.getNextProblem();
    if (problem->equality_rows.size() == 0 &&
        problem->inequality_rows.size() == 0) {
      ++num_empty;
      // TODO: update conditions for number empty to include where row length is
      // 0
    } else {

      core::FormattedProblem rf_prob = rf_.reformatProblem(*problem);
      solver_.setProblem(rf_prob.problem_matrix);
      solver_.setBasis(rf_prob.basic_variables);
      auto solution_row = solver_.solveProblem(false);

      if (!solution_row) {
        ++num_failed;
      }
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
  std::cout << "Number of empty problems: " << num_empty << std::endl;
  std::cout << "Time taken: " << time_taken_secs << " seconds" << std::endl;
  std::cout << "Num pivot row failures: " << solver_.num_pivot_row_failures
            << std::endl;
  std::cout << "Num basis failures: " << solver_.num_basis_failures
            << std::endl;

  /*
  auto problem = reader_.getNextProblem();
  core::FormattedProblem rf_prob = rf_.reformatProblem(*problem);
  solver_.setProblem(rf_prob.problem_matrix);
  solver_.setBasis(rf_prob.basic_variables);
  auto solution_row = solver_.solveProblem(true);

  // print problem
  for (std::size_t i = 0; i < rf_prob.problem_matrix.size(); ++i) {
    for (std::size_t j = 0; j < rf_prob.problem_matrix.at(0).size(); ++j) {
      std::cout << rf_prob.problem_matrix.at(i).at(j) << " ";
    }
    std::cout << std::endl;
  }
  */

  return 0;
}
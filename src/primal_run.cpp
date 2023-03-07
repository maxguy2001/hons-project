#include "primal_run.hpp"

void PrimalRun::runPrimalSolver() {

  const std::string pp = "/home/maxguy/projects/hons/hons-project/problems/"
                         "feasibility_testcases.txt";
  std::fstream filestream;
  filestream.open(pp, std::ios::in);

  // instantiate everything
  utils::ModifiedPrimalReader reader_(filestream);
  // solvers::revised_primal_simplex::RevisedPrimalSimplex solver_;
  solvers::bland_simplex::BlandPrimalSimplex solver_;
  utils::Reformatter rf_;

  // problem 2019 is first basis fault!
  int num_to_solve = 150'000;
  int num_failed = 0;
  int num_empty = 0;
  int num_sucessfully_solved = 0;
  int num_infeasible = 0;
  int num_error = 0;
  int num_didnt_converge = 0;

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
      // TODO: check if(!problem since auto)
    } else {

      core::FormattedProblem rf_prob = rf_.reformatProblem(problem.value());
      solver_.setProblem(rf_prob.problem_matrix);
      solver_.setBasis(rf_prob.basic_variables);
      // TODO: modify function inputs here to take in *problem.
      // TODO: change checks on solution row since it now returns a status
      // TODO: switch statement to find how many are sucessfully solved

      auto solution_row = solver_.solveProblem(false, *problem);
      if (solution_row == core::SolveStatus::kFeasible) {
        ++num_sucessfully_solved;
      } else if (solution_row == core::SolveStatus::kInfeasible) {
        ++num_infeasible;
      } else if (solution_row == core::SolveStatus::kError) {
        ++num_error;
      } else if (solution_row == core::SolveStatus::kDidntConverge) {
        ++num_didnt_converge;
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

  std::cout << "Number sucessfully solved " << num_sucessfully_solved
            << std::endl;
  std::cout << "Number infeasible " << num_infeasible << std::endl;
  std::cout << "Number error " << num_error << std::endl;
  std::cout << "Number didn't converge " << num_didnt_converge << std::endl;
  std::cout << "Number empty " << num_empty << std::endl;

  /*
  std::cout << "Number of problems solved: " << num_to_solve << std::endl;
  std::cout << "Number of falures: " << num_failed << std::endl;
  std::cout << "Number of empty problems: " << num_empty << std::endl;
  std::cout << "Time taken: " << time_taken_secs << " seconds" << std::endl;
  std::cout << "Num pivot row failures: " << solver_.num_pivot_row_failures_
            << std::endl;
  std::cout << "Num basis failures: " << solver_.num_basis_failures_
            << std::endl;
  std::cout << "Num already optimal: " << solver_.num_already_optimal_
            << std::endl;
  std::cout << "Num that didn't converge: " << solver_.num_not_converging_
            << std::endl;
  */
}
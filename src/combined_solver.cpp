#include "combined_solver.hpp"

void CombinedRun::runSolver(const std::string problems_filepath) {

  const std::string pp = problems_filepath;
  std::fstream filestream;
  filestream.open(pp, std::ios::in);

  // instantiate everything
  utils::ModifiedPrimalReader reader_(filestream);
  solvers::bland_simplex::BlandPrimalSimplex simplex_solver_;
  utils::PrimalReformatter prf_;
  utils::LogicalReformatter lrf_;

  // report variables
  int num_to_solve = 150'000;
  int num_empty = 0;
  int num_sucessfully_solved = 0;
  int num_infeasible = 0;
  int num_error = 0;
  int num_didnt_converge = 0;

  // to be reused
  int num_equality_constraints;
  int num_inequality_constraints;

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
    } else if (problem->equality_rows.size() == 0 &&
               problem->inequality_rows.size() == 1) {
      ++num_sucessfully_solved;
    } else if (problem->equality_rows.size() == 1 &&
               problem->inequality_rows.size() == 0) {
      ++num_sucessfully_solved;
    } else {

      // Try presolve
      core::FormattedLogicalProblem logical_problem =
          lrf_.reformatProblem(problem.value());

      num_inequality_constraints =
          static_cast<int>(problem->inequality_rows.size());
      num_equality_constraints =
          static_cast<int>(problem->equality_rows.size());

      logical_solver::Presolve presolve(
          logical_problem.problem_matrix, logical_problem.lower_bounds,
          logical_problem.upper_bounds, num_inequality_constraints,
          num_equality_constraints, false);

      presolve.applyPresolve();
      presolve.applyPostsolve();
      if (presolve.infeasible_) {
        ++num_infeasible;
      } else if (presolve.reduced_to_empty_) {
        ++num_sucessfully_solved;
      } else {
        // try simplex solver

        core::FormattedPrimalProblem rf_prob =
            prf_.reformatProblem(problem.value());
        simplex_solver_.setProblem(rf_prob.problem_matrix);
        simplex_solver_.setBasis(rf_prob.basic_variables);
        auto solve_state = simplex_solver_.solveProblem(false, *problem);

        switch (solve_state) {
        case core::SolveStatus::kDidntConverge:
          ++num_didnt_converge;
          break;

        case core::SolveStatus::kFeasible:
          ++num_sucessfully_solved;
          break;

        case core::SolveStatus::kInfeasible:
          ++num_infeasible;
          break;

        default:
          ++num_error;
          break;
        }
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

  // report results
  std::cout << "Results: " << std::endl;
  std::cout << "Number of probelms attempted: " << num_to_solve << std::endl;
  std::cout << "Time taken: " << time_taken_secs << " seconds" << std::endl;
  std::cout << "Number feasible: " << num_sucessfully_solved << std::endl;
  std::cout << "Number infeasible: " << num_infeasible << std::endl;
  std::cout << "Number of empty propblems: " << num_empty << std::endl;
  std::cout << "Number didn't converge: " << num_didnt_converge << std::endl;
  std::cout << "Number of errors: " << num_error << std::endl;
}
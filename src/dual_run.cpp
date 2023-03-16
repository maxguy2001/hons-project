#include "dual_run.hpp"

void DualRun::temp() {

  core::InputRows input;
  input.equality_rows = {{0, -1, 0, 0, 1, 0, 0, 0, 0}};
  input.inequality_rows = {{0, 1, 0, 0, 0, 0, 0, 0, 0},
                           {-1, -1, 0, 0, 0, 0, 0, 1, 0}};
  input.num_variables = 8;

  utils::DualReformatter rf_;
  core::FormattedDualProblem problem = rf_.reformatProblem(input);

  solvers::dual_simplex::DualSimplex solver_;
  solver_.setBasis(problem.basic_variables);
  solver_.setProblem(problem.problem_matrix);
  core::SolveStatus status = solver_.solveProblem(input);
}

void DualRun::printProblem(const core::InputRows input_rows) {
  std::cout << "Inequality rows: " << std::endl;
  for (std::size_t i = 0; i < input_rows.inequality_rows.size(); ++i) {
    for (std::size_t j = 0; j < input_rows.inequality_rows.at(0).size(); ++j) {
      std::cout << input_rows.inequality_rows.at(i).at(j) << " ";
    }
    std::cout << std::endl;
  }

  std::cout << std::endl;
  std::cout << "Equality Rows: " << std::endl;
  for (std::size_t i = 0; i < input_rows.equality_rows.size(); ++i) {
    for (std::size_t j = 0; j < input_rows.equality_rows.at(0).size(); ++j) {
      std::cout << input_rows.equality_rows.at(i).at(j) << " ";
    }
    std::cout << std::endl;
  }

  std::cout << std::endl;
}

void DualRun::runDualSolver() {

  const std::string pp = "/home/maxguy/projects/hons/hons-project/problems/"
                         "feasibility_testcases.txt";
  std::fstream filestream;
  filestream.open(pp, std::ios::in);

  // instantiate everything
  utils::ModifiedPrimalReader reader_(filestream);
  solvers::dual_simplex::DualSimplex solver_;
  utils::DualReformatter rf_;

  // problem 2019 is first basis fault!
  int num_to_solve = 150'000;
  int num_failed = 0;
  int num_empty = 0;
  int num_sucessfully_solved = 0;
  int num_infeasible = 0;
  int num_error = 0;
  int num_didnt_converge = 0;

  int counter = 0;
  bool debug = false;

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
    } else if (problem->equality_rows.size() == 0 &&
               problem->inequality_rows.size() == 1) {
      ++num_sucessfully_solved;
    } else if (problem->equality_rows.size() == 1 &&
               problem->inequality_rows.size() == 0) {
      ++num_sucessfully_solved;
    } else {

      core::FormattedDualProblem rf_prob = rf_.reformatProblem(problem.value());
      solver_.setProblem(rf_prob.problem_matrix);
      solver_.setBasis(rf_prob.basic_variables);
      // TODO: switch statement to find how many are sucessfully solved
      // TODO: fix floating point errors?
      auto solve_state = solver_.solveProblem(*problem);
      if (solve_state == core::SolveStatus::kFeasible) {
        ++num_sucessfully_solved;
      } else if (solve_state == core::SolveStatus::kInfeasible) {
        ++num_infeasible;
      } else if (solve_state == core::SolveStatus::kError) {
        ++num_error;
      } else if (solve_state == core::SolveStatus::kDidntConverge) {
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
}
#include "dual_run.hpp"

void DualRun::debugSingleProblem() {

  core::InputRows input;
  input.equality_rows = {{0, -1, 0, 0, 1, 0, 0, 0, 0}};
  input.inequality_rows = {{0, 1, 0, 0, 0, 0, 0, 0, 0},
                           {0, 0, 1, 0, 0, 0, 0, 0, 0},
                           {-1, 0, -1, 0, 0, 1, 0, 0, 0},
                           {-1, -1, 0, 0, 0, 0, 0, 1, 0},
                           {-1, 0, 0, 0, 0, -1, 0, 0, 1}};
  input.num_variables = 8;

  utils::DualReformatter rf_;
  core::FormattedDualProblem problem = rf_.reformatProblem(input);

  for (std::size_t i = 0; i < problem.problem_matrix.size(); ++i) {
    for (std::size_t j = 0; j < problem.problem_matrix.at(0).size(); ++j) {
      if (problem.problem_matrix.at(i).at(j) == -0) {
        std::cout << 0 << "  ";
      } else {
        std::cout << problem.problem_matrix.at(i).at(j) << "  ";
      }
    }
    std::cout << std::endl;
  }
  std::cout << std::endl;

  // solvers::dual_simplex::DualSimplex solver_;
  // solver_.setBasis(problem.basic_variables);
  // solver_.setProblem(problem.problem_matrix);
  // core::SolveStatus status = solver_.solveProblem(problem.problem_matrix);
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

std::vector<std::vector<float>>
copyMatrix(const std::vector<std::vector<float>> input) {
  std::vector<std::vector<float>> output;
  std::vector<float> temp;
  for (std::size_t i = 0; i < input.size(); ++i) {
    temp.clear();
    for (std::size_t j = 0; j < input.at(0).size(); ++j) {
      temp.push_back(input.at(i).at(j));
    }
    output.push_back(temp);
  }
  return output;
}

void DualRun::runDualSolver(const std::string input_filepath) {

  std::fstream filestream;
  filestream.open(input_filepath, std::ios::in);

  // instantiate everything
  utils::ModifiedPrimalReader reader_(filestream);
  solvers::dual_simplex::DualSimplex solver_;
  utils::DualReformatter rf_;

  // problem 2019 is first basis fault!
  int num_to_solve = 150'218;
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
      std::vector<std::vector<float>> problem_matrix_copy =
          copyMatrix(rf_prob.problem_matrix);
      auto solve_state = solver_.solveProblem(problem_matrix_copy);
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
  std::cout << "Time taken " << time_taken_secs << std::endl;
}
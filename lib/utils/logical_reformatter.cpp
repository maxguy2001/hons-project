#include "logical_reformatter.hpp"

namespace utils {

LogicalReformatter::LogicalReformatter() {}

core::FormattedLogicalProblem
LogicalReformatter::reformatProblem(const core::InputRows input_rows) {
  std::vector<std::vector<int>> problem_matrix = getProblemMatrix(input_rows);
  std::vector<double> lower_bounds = getLowerBounds(input_rows);
  std::vector<double> upper_bounds = getUpperBounds(input_rows, lower_bounds);
  core::FormattedLogicalProblem problem;
  problem.problem_matrix = problem_matrix;
  problem.lower_bounds = lower_bounds;
  problem.upper_bounds = upper_bounds;
  return problem;
}

std::vector<std::vector<int>>
LogicalReformatter::getProblemMatrix(const core::InputRows input_rows) {
  std::vector<std::vector<int>> problem_matrix;
  std::vector<int> temp_vec;

  for (std::size_t i = 0; i < input_rows.inequality_rows.size(); ++i) {
    temp_vec.clear();
    for (std::size_t j = 1; j < input_rows.inequality_rows.at(0).size(); ++j) {
      temp_vec.push_back(input_rows.inequality_rows.at(i).at(j));
    }
    problem_matrix.push_back(temp_vec);
  }

  for (std::size_t i = 0; i < input_rows.equality_rows.size(); ++i) {
    temp_vec.clear();
    // starts at 1 since constant is moved to lower bounds vector
    for (std::size_t j = 1; j < input_rows.equality_rows.at(0).size(); ++j) {
      temp_vec.push_back(input_rows.equality_rows.at(i).at(j));
    }
    problem_matrix.push_back(temp_vec);
  }

  return problem_matrix;
}

std::vector<double>
LogicalReformatter::getLowerBounds(const core::InputRows input_rows) {
  std::vector<double> lower_bounds;
  double bounds_element;

  for (std::size_t i = 0; i < input_rows.inequality_rows.size(); ++i) {
    bounds_element = -1.0 * input_rows.inequality_rows.at(i).at(0);
    lower_bounds.push_back(bounds_element);
  }

  for (std::size_t i = 0; i < input_rows.equality_rows.size(); ++i) {
    bounds_element = -1.0 * input_rows.equality_rows.at(i).at(0);
    lower_bounds.push_back(bounds_element);
  }

  return lower_bounds;
}

std::vector<double>
LogicalReformatter::getUpperBounds(const core::InputRows input_rows,
                                   const std::vector<double> lower_bounds) {
  std::vector<double> upper_bounds;

  for (std::size_t i = 0; i < input_rows.inequality_rows.size(); ++i) {
    upper_bounds.push_back(core::kFloatInfinity);
  }

  for (std::size_t j = input_rows.inequality_rows.size();
       j < input_rows.equality_rows.size() + input_rows.inequality_rows.size();
       ++j) {
    upper_bounds.push_back(lower_bounds.at(j));
  }

  return upper_bounds;
}

} // namespace utils
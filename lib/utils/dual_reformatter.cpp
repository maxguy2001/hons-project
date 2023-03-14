#include "dual_reformatter.hpp"

namespace utils {

DualRefomatter::DualRefomatter() {}

core::FormattedDualProblem
DualRefomatter::reformatProblem(const core::InputRows input_rows) {
  const std::vector<float> bounds = getBounds(input_rows);
  const std::vector<std::vector<float>> table = getTable(input_rows);
  const core::FormattedDualProblem full_table = getFullTable(table, bounds);
  return full_table;
}

std::vector<float> DualRefomatter::getBounds(const core::InputRows input_rows) {

  std::vector<float> bounds;

  // add inequality rows to bounds
  for (std::size_t i = 0; i < input_rows.inequality_rows.size(); ++i) {
    bounds.push_back(-1.0F * input_rows.inequality_rows.at(i).at(0));
  }

  // add potitive and negative of equality rows to bounds, creating equality by
  // constraint
  for (std::size_t i = 0; i < input_rows.equality_rows.size(); ++i) {
    bounds.push_back(-1.0F * input_rows.equality_rows.at(i).at(0));
  }

  for (std::size_t i = 0; i < input_rows.equality_rows.size(); ++i) {
    bounds.push_back(input_rows.equality_rows.at(i).at(0));
  }

  return bounds;
}

std::vector<std::vector<float>>
DualRefomatter::getTable(const core::InputRows input_rows) {

  std::vector<std::vector<float>> table;

  // make temporary row vector container
  std::vector<float> row_vector;

  // add inequality rows to table
  for (std::size_t i = 0; i < input_rows.inequality_rows.size(); ++i) {
    row_vector.clear();
    for (std::size_t j = 1; j < input_rows.inequality_rows.at(0).size(); ++j) {
      row_vector.push_back(input_rows.inequality_rows.at(i).at(j));
    }
    for (std::size_t j = 1; j < input_rows.inequality_rows.at(0).size(); ++j) {
      row_vector.push_back(-1.0F * input_rows.inequality_rows.at(i).at(j));
    }
    table.push_back(row_vector);
  }

  // add equality rows to table
  for (std::size_t i = 0; i < input_rows.equality_rows.size(); ++i) {
    row_vector.clear();
    for (std::size_t j = 1; j < input_rows.equality_rows.at(0).size(); ++j) {
      row_vector.push_back(input_rows.equality_rows.at(i).at(j));
    }
    for (std::size_t j = 1; j < input_rows.equality_rows.at(0).size(); ++j) {
      row_vector.push_back(-1.0F * input_rows.equality_rows.at(i).at(j));
    }
    table.push_back(row_vector);
  }

  // add negative of equality rows to table
  for (std::size_t i = 0; i < input_rows.equality_rows.size(); ++i) {
    row_vector.clear();
    for (std::size_t j = 1; j < input_rows.equality_rows.at(0).size(); ++j) {
      row_vector.push_back(-1.0F * input_rows.equality_rows.at(i).at(j));
    }
    for (std::size_t j = 1; j < input_rows.equality_rows.at(0).size(); ++j) {
      row_vector.push_back(input_rows.equality_rows.at(i).at(j));
    }
    table.push_back(row_vector);
  }
  return table;
}

core::FormattedDualProblem
DualRefomatter::getFullTable(const std::vector<std::vector<float>> input_table,
                             const std::vector<float> bounds) {

  // container for full table
  std::vector<std::vector<float>> full_table;

  // temporary row vector for use in making table
  std::vector<float> row_vector;
  const int row_length = input_table.size() + input_table.at(0).size() + 1;

  // add objective row
  row_vector.push_back(1.0F);
  for (std::size_t i = 0; i < row_length; ++i) {
    row_vector.push_back(0.0F);
  }
  full_table.push_back(row_vector);
  row_vector.clear();

  int index_1_position = 0;
  for (std::size_t i = 0; i < input_table.size(); ++i) {
    row_vector.clear();
    row_vector.push_back(0);
    for (std::size_t j = 0; j < input_table.at(0).size(); ++j) {
      row_vector.push_back(input_table.at(i).at(j));
    }
    for (std::size_t j = 0; j < input_table.size(); ++j) {
      if (j == index_1_position) {
        row_vector.push_back(1);
      } else {
        row_vector.push_back(0);
      }
    }
    row_vector.push_back(bounds.at(i));
    full_table.push_back(row_vector);
  }

  core::FormattedDualProblem formatted_problem;
  formatted_problem.problem_matrix = full_table;
  // TODO: figure out basic variables here
  // formatted_problem.basic_variables;

  return formatted_problem;
}

} // namespace utils
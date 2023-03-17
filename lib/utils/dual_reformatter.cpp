#include "dual_reformatter.hpp"
#include <iostream>

namespace utils {

DualReformatter::DualReformatter() {}

core::FormattedDualProblem
DualReformatter::reformatProblem(const core::InputRows input_rows) {
  const std::vector<float> bounds = getBounds(input_rows);
  const std::vector<std::vector<float>> table = getTable(input_rows);
  const std::vector<std::vector<float>> full_table =
      constructFullTable(table, bounds);
  const std::vector<int> basic_variables =
      getBasicVariables(full_table, bounds);

  core::FormattedDualProblem final_problem;
  final_problem.problem_matrix = full_table;
  final_problem.basic_variables = basic_variables;
  return final_problem;
}

std::vector<float>
DualReformatter::getBounds(const core::InputRows input_rows) {

  std::vector<float> bounds;

  // add inequality rows to bounds
  for (std::size_t i = 0; i < input_rows.inequality_rows.size(); ++i) {
    bounds.push_back(input_rows.inequality_rows.at(i).at(0));
  }

  // add potitive and negative of equality rows to bounds, creating equality by
  // constraint
  for (std::size_t i = 0; i < input_rows.equality_rows.size(); ++i) {
    bounds.push_back(input_rows.equality_rows.at(i).at(0));
  }

  for (std::size_t i = 0; i < input_rows.equality_rows.size(); ++i) {
    bounds.push_back(-1.0F * input_rows.equality_rows.at(i).at(0));
  }

  return bounds;
}

std::vector<std::vector<float>>
DualReformatter::getTable(const core::InputRows input_rows) {

  // intitialise table container
  std::vector<std::vector<float>> table;

  // make temporary row vector container
  std::vector<float> row_vector;

  // add inequality rows to table
  for (std::size_t i = 0; i < input_rows.inequality_rows.size(); ++i) {
    row_vector.clear();
    for (std::size_t j = 1; j < input_rows.inequality_rows.at(0).size(); ++j) {
      row_vector.push_back(-1.0F * input_rows.inequality_rows.at(i).at(j));
    }
    for (std::size_t j = 1; j < input_rows.inequality_rows.at(0).size(); ++j) {
      row_vector.push_back(input_rows.inequality_rows.at(i).at(j));
    }
    table.push_back(row_vector);
  }

  // add equality rows (and their negative) to table
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
  return table;
}

std::vector<std::vector<float>> DualReformatter::constructFullTable(
    const std::vector<std::vector<float>> primal_table,
    const std::vector<float> bounds) {

  // empty container to hold full problem table
  std::vector<std::vector<float>> problem_table;
  const std::uint32_t row_length =
      primal_table.size() + primal_table.at(0).size() + 2;

  // temporary container to hold rows as they are created
  std::vector<float> temp_row;

  // add objective row
  temp_row.push_back(1);
  for (std::size_t i = 1; i < row_length; ++i) {
    temp_row.push_back(0);
  }
  problem_table.push_back(temp_row);
  temp_row.clear();

  // to keep track of slack being added to dual table
  std::size_t identity_1_position = 0;

  for (std::size_t i = 0; i < primal_table.size(); ++i) {
    temp_row.clear();
    temp_row.push_back(0);
    // add primal row
    for (std::size_t j = 0; j < primal_table.at(0).size(); ++j) {
      temp_row.push_back(primal_table.at(i).at(j));
    }
    // add slack
    for (std::size_t j = 0; j < primal_table.size(); ++j) {
      if (j == identity_1_position) {
        temp_row.push_back(-1);
      } else {
        temp_row.push_back(0);
      }
    }
    ++identity_1_position;
    temp_row.push_back(bounds.at(i));
    problem_table.push_back(temp_row);
  }

  return problem_table;
}

std::vector<int> DualReformatter::getBasicVariables(
    const std::vector<std::vector<float>> full_table,
    const std::vector<float> bounds) {

  const int num_basic_vars = bounds.size();
  const int num_columns = full_table.at(0).size();
  const int basis_start_point = num_columns - num_basic_vars - 1;
  std::vector<int> basic_variables;
  for (std::size_t i = 0; i < num_basic_vars; ++i) {
    basic_variables.push_back(basis_start_point + i);
  }
  return basic_variables;
}

} // namespace utils
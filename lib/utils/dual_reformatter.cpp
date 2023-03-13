#include "dual_reformatter.hpp"

namespace utils {

DualRefomatter::DualRefomatter() {}

core::FormattedDualProblem
DualRefomatter::reformatProblem(const core::InputRows input_rows) {
  const std::vector<float> dual_bounds = getDualBounds(input_rows);
  const std::vector<std::vector<float>> dual_table = getDualTable(input_rows);
  const std::vector<float> objective_row =
      getObjectiveRow(dual_bounds, dual_table);
  const core::FormattedDualProblem full_table =
      getFullTable(dual_table, objective_row);
  return full_table;
}

std::vector<float> getDualBounds(const core::InputRows input_rows) {

  const float negative_one = -1.0;
  std::vector<float> dual_bounds;

  // add inequality rows to bounds
  for (std::size_t i = 0; i < input_rows.inequality_rows.size(); ++i) {
    dual_bounds.push_back(negative_one *
                          input_rows.inequality_rows.at(i).at(0));
  }

  // add potitive and negative of equality rows to bounds, creating equality by
  // constraint
  for (std::size_t i = 0; i < input_rows.equality_rows.size(); ++i) {
    dual_bounds.push_back(negative_one * input_rows.equality_rows.at(i).at(0));
  }

  for (std::size_t i = 0; i < input_rows.equality_rows.size(); ++i) {
    dual_bounds.push_back(input_rows.equality_rows.at(i).at(0));
  }

  return dual_bounds;
}

std::vector<std::vector<float>>
DualRefomatter::getDualTable(const core::InputRows input_rows) {

  std::vector<std::vector<float>> dual_table;

  // make temporary row vector container
  std::vector<float> row_vector;
  int rowlength;
  const float negative_one = -1.0;

  // add inequality rows to table
  for (std::size_t i = 0; i < input_rows.inequality_rows.size(); ++i) {
    // remove first element of row vector
    row_vector = input_rows.inequality_rows.at(i);
    row_vector.erase(row_vector.begin());
    // add negative of ineqality constriaint to row
    rowlength = row_vector.size();
    for (std::size_t j = 0; j < rowlength; ++j) {
      row_vector.push_back(negative_one * row_vector.at(j));
    }
    dual_table.push_back(row_vector);
    row_vector.clear();
  }

  // add equality rows (and their negative) to table
  for (std::size_t i = 0; i < input_rows.equality_rows.size(); ++i) {
    // remove first element of row vector
    row_vector = input_rows.equality_rows.at(i);
    row_vector.erase(row_vector.begin());
    // add negative of eqality constriaint to row
    rowlength = row_vector.size();
    for (std::size_t j = 0; j < rowlength; ++j) {
      row_vector.push_back(negative_one * row_vector.at(j));
    }
    dual_table.push_back(row_vector);
    row_vector.clear();
  }

  for (std::size_t i = 0; i < input_rows.equality_rows.size(); ++i) {
    // remove first element of row vector
    row_vector = input_rows.equality_rows.at(i);
    row_vector.erase(row_vector.begin());
    for (std::size_t j = 0; j < row_vector.size(); ++j) {
      row_vector.at(j) = negative_one * row_vector.at(j);
    }
    // add negative of eqality constriaint to row
    rowlength = row_vector.size();
    for (std::size_t j = 0; j < rowlength; ++j) {
      row_vector.push_back(negative_one * row_vector.at(j));
    }
    dual_table.push_back(row_vector);
    row_vector.clear();
  }

  return dual_table;
}

std::vector<float> DualRefomatter::getObjectiveRow(
    const std::vector<float> dual_bounds,
    const std::vector<std::vector<float>> dual_table) {

  // define length of objectve row
  const uint32_t row_length =
      dual_table.size() + 2 * dual_table.at(0).size() + 1;

  // initialiase and add first element of objective row
  std::vector<float> dual_objective_row;
  dual_objective_row.push_back(1);

  const float negative_one = -1.0;

  // add primal bounds to dual objective row
  for (std::size_t i = 0; i < dual_bounds.size(); ++i) {
    dual_objective_row.push_back(negative_one * dual_bounds.at(i));
  }

  // add the neccesary number of zeros on to the end
  const uint32_t num_zero_entries = row_length - dual_objective_row.size();
  for (size_t i = 0; i < num_zero_entries; ++i) {
    dual_objective_row.push_back(0);
  }

  // allow for zero bounds on dual table
  dual_objective_row.push_back(0);

  return dual_objective_row;
}

core::FormattedDualProblem
DualRefomatter::getFullTable(const std::vector<std::vector<float>> dual_table,
                             const std::vector<float> objective_row) {

  std::vector<std::vector<float>> dual_table;
  dual_table.push_back(objective_row);

  const std::uint32_t number_positive_rows = dual_table.at(0).size();
  const std::uint32_t row_length = objective_row.size() - 1;

  // to keep track of slack being added to dual table
  std::size_t identity_1_position = 0;

  // temporary container to hold rows as they are created and slack matrix
  // temporary colmns to be added
  std::vector<float> temp_row;
  std::uint32_t num_slack_columns;

  // adding the negative primal table entries (transposed)
  for (std::size_t i = 0; i < number_positive_rows; ++i) {
    temp_row.push_back(0);

    // add primal transposed entries
    for (std::size_t j = 0; j < dual_table.size(); ++j) {
      temp_row.push_back(dual_table.at(j).at(i));
    }

    // add slack
    num_slack_columns = row_length - temp_row.size();
    for (std::size_t j = 0; j < num_slack_columns; ++j) {
      if (j == identity_1_position) {
        temp_row.push_back(1);
      } else {
        temp_row.push_back(0);
      }
    }

    // add zero bounds column
    temp_row.push_back(0);

    // add row, increment identity position and clear temporary row
    dual_table.push_back(temp_row);
    ++identity_1_position;
    temp_row.clear();
  }

  const float negative_one = -1.0;
  // adding the positive primal table entries (transposed)
  for (std::size_t i = 0; i < number_positive_rows; ++i) {
    temp_row.push_back(0);

    // add primal transposed entries
    for (std::size_t j = 0; j < dual_table.size(); ++j) {
      temp_row.push_back(negative_one * dual_table.at(j).at(i));
    }

    // add slack
    num_slack_columns = row_length - temp_row.size();
    for (std::size_t j = 0; j < num_slack_columns; ++j) {
      if (j == identity_1_position) {
        temp_row.push_back(1);
      } else {
        temp_row.push_back(0);
      }
    }

    // add zero bounds column
    temp_row.push_back(0);

    // add row, increment identity position and clear temporary row
    dual_table.push_back(temp_row);
    ++identity_1_position;
    temp_row.clear();
  }

  // Get intial basis
  std::vector<int> initial_basis;
  for (std::size_t i = dual_table.size() + 1; i < objective_row.size() - 2;
       ++i) {
    initial_basis.push_back(i);
  }

  core::FormattedPrimalProblem formatted_problem;
  formatted_problem.problem_matrix = dual_table;
  formatted_problem.basic_variables = initial_basis;

  return formatted_problem;
}

} // namespace utils
#include "reformatter.hpp"

namespace utils {

Reformatter::Reformatter() {}

core::FormattedProblem
Reformatter::reformatProblem(const core::InputRows input_rows) {
  const std::vector<float> primal_bounds = getPrimalBounds(input_rows);
  const std::vector<std::vector<float>> primal_table =
      getPrimalTable(input_rows);
  const std::vector<float> dual_objective_row =
      getDualObjectiveRow(primal_bounds, primal_table);
  const core::FormattedProblem dual_table =
      getFullDualTable(primal_table, dual_objective_row);
  return dual_table;
}

std::vector<float>
Reformatter::getPrimalBounds(const core::InputRows input_rows) {

  const float negative_one = -1.0;
  std::vector<float> primal_bounds;

  // add inequality rows to bounds
  for (std::size_t i = 0; i < input_rows.inequality_rows.size(); ++i) {
    primal_bounds.push_back(negative_one *
                            input_rows.inequality_rows.at(i).at(0));
  }

  // add potitive and negative of equality rows to bounds, creating equality by
  // constraint
  for (std::size_t i = 0; i < input_rows.equality_rows.size(); ++i) {
    primal_bounds.push_back(negative_one *
                            input_rows.equality_rows.at(i).at(0));
  }

  for (std::size_t i = 0; i < input_rows.equality_rows.size(); ++i) {
    primal_bounds.push_back(input_rows.equality_rows.at(i).at(0));
  }

  return primal_bounds;
}

std::vector<std::vector<float>>
Reformatter::getPrimalTable(const core::InputRows input_rows) {

  std::vector<std::vector<float>> primal_table;

  // make temporary row vector container
  std::vector<float> row_vector;
  const float negative_one = -1.0;

  // add inequality rows to table
  for (std::size_t i = 0; i < input_rows.inequality_rows.size(); ++i) {
    // remove first element of row vector
    row_vector = input_rows.inequality_rows.at(i);
    row_vector.erase(row_vector.begin());
    primal_table.push_back(row_vector);
    row_vector.clear();
  }

  // add equality rows (and their negative) to table
  for (std::size_t i = 0; i < input_rows.equality_rows.size(); ++i) {
    // remove first element of row vector
    row_vector = input_rows.equality_rows.at(i);
    row_vector.erase(row_vector.begin());
    primal_table.push_back(row_vector);
    row_vector.clear();
  }

  for (std::size_t i = 0; i < input_rows.equality_rows.size(); ++i) {
    // remove first element of row vector
    row_vector = input_rows.equality_rows.at(i);
    row_vector.erase(row_vector.begin());
    for (std::size_t j = 0; j < row_vector.size(); ++j) {
      row_vector.at(j) = negative_one * row_vector.at(j);
    }
    primal_table.push_back(row_vector);
    row_vector.clear();
  }

  return primal_table;
}

std::vector<float> Reformatter::getDualObjectiveRow(
    const std::vector<float> primal_bounds,
    const std::vector<std::vector<float>> primal_table) {

  // define length of objectve row
  const uint32_t row_length =
      primal_table.size() + 2 * primal_table.at(0).size() + 1;

  // initialiase and add first element of objective row
  std::vector<float> dual_objective_row;
  dual_objective_row.push_back(1);

  const float negative_one = -1.0;

  // add primal bounds to dual objective row
  for (std::size_t i = 0; i < primal_bounds.size(); ++i) {
    dual_objective_row.push_back(negative_one * primal_bounds.at(i));
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

core::FormattedProblem Reformatter::getFullDualTable(
    const std::vector<std::vector<float>> primal_table,
    const std::vector<float> objective_row) {

  std::vector<std::vector<float>> dual_table;
  dual_table.push_back(objective_row);

  const std::uint32_t number_positive_rows = primal_table.at(0).size();
  const std::uint32_t row_length = objective_row.size() - 1;

  // to keep track of slack being added to dual table
  std::size_t identity_1_position = 0;

  // vector to keep track of initial basis
  std::vector<int> initial_basis;

  // temporary container to hold rows as they are created and slack matrix
  // temporary colmns to be added
  std::vector<float> temp_row;
  std::uint32_t num_slack_columns;

  // adding the negative primal table entries (transposed)
  for (std::size_t i = 0; i < number_positive_rows; ++i) {
    temp_row.push_back(0);

    // add primal transposed entries
    for (std::size_t j = 0; j < primal_table.size(); ++j) {
      temp_row.push_back(primal_table.at(j).at(i));
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
    initial_basis.push_back(primal_table.size() + identity_1_position);
    ++identity_1_position;
    temp_row.clear();
  }

  const float negative_one = -1.0;
  // adding the positive primal table entries (transposed)
  for (std::size_t i = 0; i < number_positive_rows; ++i) {
    temp_row.push_back(0);

    // add primal transposed entries
    for (std::size_t j = 0; j < primal_table.size(); ++j) {
      temp_row.push_back(negative_one * primal_table.at(j).at(i));
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
    initial_basis.push_back(primal_table.size() + identity_1_position);
    ++identity_1_position;
    temp_row.clear();
  }

  core::FormattedProblem formatted_problem;
  formatted_problem.problem_matrix = dual_table;
  formatted_problem.basic_variables = initial_basis;

  return formatted_problem;
}

} // namespace utils
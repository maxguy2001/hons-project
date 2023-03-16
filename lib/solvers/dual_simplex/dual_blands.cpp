#include "dual_blands.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <iterator>

namespace solvers::dual_simplex {

DualSimplex::DualSimplex() {}

void DualSimplex::setProblem(const std::vector<std::vector<float>> table) {
  // clear old data
  table_.clear();

  // import new data
  for (size_t i = 0; i < table.size(); ++i) {
    table_.push_back(table.at(i));
  }
}

void DualSimplex::setBasis(const std::vector<int> basis) {
  // clear old data
  basis_.clear();

  // import new data
  for (size_t i = 0; i < basis.size(); ++i) {
    basis_.push_back(basis.at(i));
  }
}

std::vector<float> DualSimplex::extractColumnFromTable(const int column_index) {
  std::vector<float> column;
  float table_element;
  for (size_t i = 0; i < table_.size(); ++i) {
    table_element = table_.at(i).at(column_index);
    column.push_back(table_element);
  }
  return column;
}

int DualSimplex::getPivotColumnIndex(const int pivot_row_index) {

  // get objective function
  const std::vector<float> pivot_row = table_.at(pivot_row_index);

  for (std::size_t i = 1; i < pivot_row.size(); ++i) {
    if (pivot_row.at(i) < 0 &&
        std::find(basis_.begin(), basis_.end(), i) == basis_.end()) {
      return i;
    }
  }
  // if already optimal
  return -1;
}

int DualSimplex::getPivotRowIndex() {

  // extract primal bounds vector
  const std::vector<float> bounds =
      extractColumnFromTable(table_.at(0).size() - 1);

  for (std::size_t i = 1; i < bounds.size(); ++i) {
    if (bounds.at(i) < 0) {
      return i;
    }
  }
  return -1;
}

bool DualSimplex::switchBasis(const int pivot_row_index,
                              const int pivot_column_index) {

  // check fail state for now
  if (basis_.size() < pivot_row_index) {
    return false;
  }
  // update basis
  basis_.at(pivot_row_index - 1) = pivot_column_index;
  return true;
}

void DualSimplex::constructNewTable(const int pivot_row_index,
                                    const int pivot_column_index) {
  // define new table
  std::vector<std::vector<float>> new_table;

  // define containers and floats to be used for new rows
  std::vector<float> new_row;
  std::vector<float> old_row;
  float new_elem;
  float product;

  // defining pivot vectors
  const float pivot_value = table_.at(pivot_row_index).at(pivot_column_index);
  const std::vector<float> pivot_column =
      extractColumnFromTable(pivot_column_index);
  const std::vector<float> pivot_row = table_.at(pivot_row_index);

  // iterate over elements to create new table
  for (size_t i = 0; i < table_.size(); ++i) {
    // clear container for reuse
    new_row.clear();
    old_row.clear();

    // case if we are adding pivot row
    if (i == pivot_row_index) {
      // divide all elements by pivot value and add to new table
      for (size_t j = 0; j < pivot_row.size(); ++j) {
        new_elem = pivot_row.at(j) / pivot_value;
        new_row.push_back(new_elem);
      }
      // add new row to new table
      new_table.push_back(new_row);
    }
    // case if pivot row element is already zero
    else if (std::fabs(pivot_column.at(i)) < core::kEpsilon) {
      // just add row to new table
      for (size_t j = 0; j < pivot_row.size(); ++j) {
        new_elem = table_.at(i).at(j);
        new_row.push_back(new_elem);
      }
      // add new row to new table
      new_table.push_back(new_row);

    } else {
      // get old row
      for (size_t j = 0; j < pivot_row.size(); ++j) {
        old_row.push_back(table_.at(i).at(j));
      }

      // create new row
      product = pivot_value * pivot_column.at(i);

      for (size_t j = 0; j < old_row.size(); ++j) {
        new_elem = (old_row.at(j) * (product / pivot_column.at(i))) -
                   (pivot_row.at(j) * (product / pivot_value));
        new_row.push_back(new_elem);
      }
      // add new row to new table
      new_table.push_back(new_row);
    }
  }

  // replace table with new one
  table_.clear();
  for (size_t i = 0; i < new_table.size(); ++i) {
    table_.push_back(new_table.at(i));
  }
}

bool DualSimplex::checkOptimality() {

  // extract primal bounds vector
  const std::vector<float> bounds =
      extractColumnFromTable(table_.at(0).size() - 1);

  for (size_t i = 0; i < bounds.size(); ++i) {
    if (bounds.at(i) < 0) {
      return false;
    }
  }
  return true;
}

std::vector<float>
DualSimplex::extractSolution(const core::InputRows original_problem) {
  int num_primal_variables;
  if (original_problem.equality_rows.size() == 0) {
    num_primal_variables = original_problem.inequality_rows.at(0).size();
  } else {
    num_primal_variables = original_problem.equality_rows.at(0).size();
  }

  std::vector<float> bounds = extractColumnFromTable(table_.at(0).size() - 1);

  std::vector<float> extended_x_vals;
  for (std::size_t i = 0; i < num_primal_variables * 2; ++i) {
    extended_x_vals.push_back(0);
  }
  for (std::size_t i = 0; i < basis_.size(); ++i) {
    if (basis_.at(i) < 2 * num_primal_variables) {
      extended_x_vals.at(basis_.at(i)) = bounds.at(i + 1);
    }
  }

  std::vector<float> x_vals;
  for (std::size_t i = 0; i < num_primal_variables; ++i) {
    x_vals.push_back(extended_x_vals.at(i) -
                     extended_x_vals.at(i + num_primal_variables));
  }

  return x_vals;
}

core::SolveStatus DualSimplex::verifySolution(core::InputRows original_problem,
                                              std::vector<float> solution_row) {

  std::vector<float> x = extractSolution(original_problem);
  solution_ = x;
  const int num_primal_variables = original_problem.num_variables - 1;

  // check inequalities hold
  float total = 0;
  for (std::size_t i = 0; i < original_problem.inequality_rows.size(); ++i) {
    total += original_problem.inequality_rows.at(i).at(0);
    for (std::size_t j = 0; j < num_primal_variables + 1; ++j) {
      total += original_problem.inequality_rows.at(i).at(j) * x.at(j);
    }
    if (total < -core::kEpsilon) {
      return core::SolveStatus::kError;
    }
    total = 0;
  }

  // check equalities hold
  total = 0;
  for (std::size_t i = 0; i < original_problem.equality_rows.size(); ++i) {
    total += original_problem.equality_rows.at(i).at(0);
    for (std::size_t j = 0; j < num_primal_variables + 1; ++j) {
      total += original_problem.equality_rows.at(i).at(j) * x.at(j);
    }
    if (std::fabs(total) > core::kEpsilon) {
      return core::SolveStatus::kError;
    }
    total = 0;
  }

  return core::SolveStatus::kFeasible;
}

core::SolveStatus
DualSimplex::solveProblem(const core::InputRows original_problem) {

  solution_.clear();
  for (size_t i = 0; i < core::kMaxIterations; ++i) {
    // we now get pivot row index first!
    int pivot_row_index = getPivotRowIndex();
    if (pivot_row_index == -1) {
      return core::SolveStatus::kInfeasible;
    }

    int pivot_column_index = getPivotColumnIndex(pivot_row_index);
    if (pivot_column_index == -1) {
      core::SolveStatus solution_status =
          verifySolution(original_problem, table_.at(0));
      return solution_status;
    }

    bool is_basis_switch_successful =
        switchBasis(pivot_row_index, pivot_column_index);
    if (!is_basis_switch_successful) {
      return core::SolveStatus::kError;
    }
    constructNewTable(pivot_row_index, pivot_column_index);

    if (checkOptimality()) {
      core::SolveStatus solution_status =
          verifySolution(original_problem, table_.at(0));
      return solution_status;
    }
  }
  return core::SolveStatus::kDidntConverge;
}

} // namespace solvers::dual_simplex

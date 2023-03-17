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

std::vector<float> DualSimplex::extractSolution() {

  const int row_length = table_.at(0).size();

  std::vector<float> solution_vector;
  // make vector of zeros
  for (std::size_t i = 0; i < row_length; ++i) {
    solution_vector.push_back(0);
  }

  const std::vector<float> bounds =
      extractColumnFromTable(table_.at(0).size() - 1);
  // add any nonzero variables that appear in the basis
  for (std::size_t i = 0; i < basis_.size(); ++i) {
    solution_vector.at(basis_.at(i)) = bounds.at(i + 1);
  }
  return solution_vector;
}

core::SolveStatus DualSimplex::verifySolution(
    const std::vector<std::vector<float>> original_formatted_problem) {

  const std::vector<float> extracted_solutions = extractSolution();
  solution_ = extracted_solutions;

  const int bounds_column_index = original_formatted_problem.at(0).size() - 1;
  std::vector<float> bounds;
  float bounds_element;
  for (size_t i = 0; i < original_formatted_problem.size(); ++i) {
    bounds_element = table_.at(i).at(bounds_column_index);
    bounds.push_back(bounds_element);
  }

  // check constraints hold
  float total = 0;
  for (std::size_t i = 0; i < original_formatted_problem.size(); ++i) {
    for (std::size_t j = 0; j < original_formatted_problem.at(0).size() - 1;
         ++j) {
      total +=
          original_formatted_problem.at(i).at(j) * extracted_solutions.at(j);
    }
    // total += bounds.at(i);
    // if (std::fabs(total) - bounds.at(i) > core::kEpsilon) {
    if (total - bounds.at(i) > core::kEpsilon) {

      return core::SolveStatus::kError;
    }
    total = 0;
  }
  return core::SolveStatus::kFeasible;
}

core::SolveStatus DualSimplex::solveProblem(
    const std::vector<std::vector<float>> original_formatted_problem) {

  solution_.clear();
  for (size_t i = 0; i < core::kMaxIterations; ++i) {
    // we now get pivot row index first!
    int pivot_row_index = getPivotRowIndex();
    if (pivot_row_index == -1) {
      core::SolveStatus solution_status =
          verifySolution(original_formatted_problem);
      return solution_status;
    }

    int pivot_column_index = getPivotColumnIndex(pivot_row_index);
    if (pivot_column_index == -1) {
      core::SolveStatus solution_status =
          verifySolution(original_formatted_problem);
      if (solution_status == core::SolveStatus::kFeasible) {
        return solution_status;
      }
      return core::SolveStatus::kInfeasible;
    }

    bool is_basis_switch_successful =
        switchBasis(pivot_row_index, pivot_column_index);
    if (!is_basis_switch_successful) {
      return core::SolveStatus::kError;
    }
    constructNewTable(pivot_row_index, pivot_column_index);

    if (checkOptimality()) {
      core::SolveStatus solution_status =
          verifySolution(original_formatted_problem);
      return solution_status;
    }
  }
  return core::SolveStatus::kDidntConverge;
}

} // namespace solvers::dual_simplex

#include "simplex.hpp"
#include "../core/consts.hpp"
#include <algorithm>
#include <iostream>
#include <iterator>

namespace revised_primal_simplex {

RevisedPrimalSimplex::RevisedPrimalSimplex() {}

void RevisedPrimalSimplex::setProblem(
    const std::vector<std::vector<float>> table) {
  // clear old data
  table_.clear();

  // import new data
  for (size_t i = 0; i < table.size(); ++i) {
    table_.push_back(table.at(i));
  }
}

void RevisedPrimalSimplex::setBasis(const std::vector<int> basis) {
  // clear old data
  basis_.clear();

  // import new data
  for (size_t i = 0; i < basis.size(); ++i) {
    basis_.push_back(basis.at(i));
  }
}

std::vector<float>
RevisedPrimalSimplex::extractColumnFromTable(const int column_index) {
  std::vector<float> column;
  float table_element;
  for (size_t i = 0; i < table_.size(); ++i) {
    table_element = table_.at(i).at(column_index);
    column.push_back(table_element);
  }
  return column;
}

int RevisedPrimalSimplex::getPivotColumnIndexFixed() {

  // get objective function
  const std::vector<float> objective_function = table_.at(0);

  // set initial point for pivot column index
  int pivot_column_index;
  float min_value;
  for (std::size_t i = 1; i < objective_function.size(); ++i) {
    if (std::find(basis_.begin(), basis_.end(), i) == basis_.end()) {
      pivot_column_index = i;
      min_value = objective_function.at(i);
    }
  }

  // find pivot column index
  for (std::size_t i = 1; i < objective_function.size(); ++i) {
    if (std::find(basis_.begin(), basis_.end(), i) == basis_.end() &&
        objective_function.at(i) < min_value) {
      pivot_column_index = i;
      min_value = objective_function.at(i);
    }
  }
  return pivot_column_index;
}

int RevisedPrimalSimplex::getPivotRowIndex(const int pivot_column_index) {

  // extract pivot and bounds columns
  const std::vector<float> pivot_column =
      extractColumnFromTable(pivot_column_index);
  const std::vector<float> bounds_column =
      extractColumnFromTable(table_.at(0).size() - 1);

  const auto max_value =
      std::max_element(pivot_column.begin(), pivot_column.end());
  const int pivot_row_index = std::distance(pivot_column.begin(), max_value);

  if (pivot_row_index <= 0) {
    return -1;
  }

  return pivot_row_index;
}

bool RevisedPrimalSimplex::switchBasis(const int pivot_row_index,
                                       const int pivot_column_index,
                                       const bool verbosity) {
  // TODO: fix this monstrosity
  if (verbosity) {
    // print basis
    std::cout << "Basis: " << std::endl;
    for (std::size_t i = 0; i < basis_.size(); ++i) {
      std::cout << basis_.at(i) << " ";
    }
    std::cout << std::endl;

    // print problem size
    std::cout << "Problem size: " << table_.size() << " rows, "
              << table_.at(0).size() << " columns" << std::endl;

    // print pivot row index
    std::cout << "Pivot row index: " << pivot_row_index << std::endl;

    // print pivot column index
    std::cout << "Pivot column index: " << pivot_column_index << std::endl;
  }

  // check fail state for now
  if (basis_.size() < pivot_row_index) {
    return false;
  }
  // update basis
  basis_.at(pivot_row_index - 1) = pivot_column_index;
  return true;
}

void RevisedPrimalSimplex::constructNewTable(const int pivot_row_index,
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
    else if (std::abs(pivot_column.at(i)) < core::kEpsilon) {
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

bool RevisedPrimalSimplex::checkOptimality() {

  const std::vector<float> objective_row = table_.at(0);

  for (size_t i = 0; i < objective_row.size(); ++i) {
    if (objective_row.at(i) < 0) {
      return false;
    }
  }
  return true;
}

void RevisedPrimalSimplex::printObjectiveRow() {
  std::vector<float> obj_row = table_.at(0);
  for (std::size_t i = 0; i < obj_row.size(); ++i) {
    std::cout << obj_row.at(i) << " ";
  }
  std::cout << std::endl;
}

std::optional<std::vector<float>>
RevisedPrimalSimplex::solveProblem(const bool run_verbose) {

  for (size_t i = 0; i < core::kMaxIterations; ++i) {
    int pivot_column_index = getPivotColumnIndexFixed();
    int pivot_row_index = getPivotRowIndex(pivot_column_index);
    if (pivot_row_index == -1) {
      ++num_pivot_row_failures;
      return std::nullopt;
    }
    // TODO: fix this function (guarantee that pivot_row_index is not >
    // len(basis)...)
    bool is_basis_switch_successful =
        switchBasis(pivot_row_index, pivot_column_index, run_verbose);
    if (!is_basis_switch_successful) {
      ++num_basis_failures;
      return std::nullopt;
    }
    constructNewTable(pivot_row_index, pivot_column_index);

    if (checkOptimality()) {
      return table_.at(0);
    }
  }
  return std::nullopt;
} // namespace revised_primal_simplex

} // namespace revised_primal_simplex
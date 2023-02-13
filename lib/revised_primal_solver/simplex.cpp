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

int RevisedPrimalSimplex::getPivotColumnIndex() {

  // get objective function from table and make copy
  const std::vector<float> objective_function = table_.at(0);
  std::vector<float> objective_values;
  std::copy(objective_function.begin(), objective_function.end(),
            std::back_inserter(objective_values));

  // make vector containing corresponding index for objective_values
  std::vector<int> index;
  for (size_t i = 0; i < objective_values.size(); ++i) {
    index.push_back(i);
  }

  // make copy of basis
  std::vector<int> basis_copy;
  std::copy(basis_.begin(), basis_.end(), std::back_inserter(basis_copy));

  // sort basis copy descending
  std::sort(basis_copy.begin(), basis_copy.end(), std::greater<int>());

  // remove basic variables from objective_values and index
  int index_to_remove;
  for (size_t i = 0; i < basis_copy.size(); ++i) {
    index_to_remove = basis_copy.at(i);
    objective_values.erase(objective_values.begin() + index_to_remove);
    index.erase(index.begin() + index_to_remove);
  }

  // locate and return smallest nonbasic value in objective function
  auto min_value =
      std::min_element(objective_values.begin(), objective_values.end());
  int min_index;
  for (size_t i = 0; i < objective_values.size(); ++i) {
    if (std::abs(objective_values.at(i) - *min_value) < core::kEpsilon) {
      min_index = i;
    }
  }

  return index.at(min_index);
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

void RevisedPrimalSimplex::switchBasis(const int pivot_row_index,
                                       const int pivot_column_index) {
  // update basis
  basis_.at(pivot_row_index - 1) = pivot_column_index;
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

std::optional<std::vector<float>> RevisedPrimalSimplex::solveProblem() {

  for (size_t i = 0; i < core::kMaxIterations; ++i) {
    // TODO: fix segmentation fault
    int pivot_column_index = getPivotColumnIndex();
    int pivot_row_index = getPivotRowIndex(pivot_column_index);
    if (pivot_row_index == -1) {
      printObjectiveRow();
      return table_.at(0);
    }
    switchBasis(pivot_row_index, pivot_column_index);
    constructNewTable(pivot_row_index, pivot_column_index);

    if (checkOptimality()) {
      printObjectiveRow();
      return table_.at(0);
    }
  }
  return std::nullopt;
}

} // namespace revised_primal_simplex
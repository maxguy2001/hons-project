#include "main.hpp"
#include "core/consts.hpp"
#include <algorithm>
#include <iostream>
#include <iterator>

namespace primal_simplex {

PrimalSimplex::PrimalSimplex() {}

void PrimalSimplex::setProblem(const std::vector<std::vector<float>> table) {
  // clear old data
  table_.clear();

  // import new data
  for (size_t i = 0; i < table.size(); ++i) {
    table_.push_back(table.at(i));
  }
}

void PrimalSimplex::setBasis(const std::vector<int> basis) {
  // clear old data
  basis_.clear();

  // import new data
  for (size_t i = 0; i < basis.size(); ++i) {
    basis_.push_back(basis.at(i));
  }
}

std::vector<float>
PrimalSimplex::extractColumnFromTable(const int column_index) {
  std::vector<float> column;
  float table_element;
  for (size_t i = 0; i < table_.size(); ++i) {
    table_element = table_.at(i).at(column_index);
    column.push_back(table_element);
  }
  return column;
}

int PrimalSimplex::getPivotColumnIndex() {

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

int PrimalSimplex::getPivotRowIndex(const int pivot_column_index) {

  // extract pivot and bounds columns
  const std::vector<float> pivot_column =
      extractColumnFromTable(pivot_column_index);
  const std::vector<float> bounds_column =
      extractColumnFromTable(table_.at(0).size() - 1);

  // conduct ratios test
  std::vector<float> ratios;
  float ratio;
  for (size_t i = 0; i < pivot_column.size(); ++i) {
    if (pivot_column.at(i) <= 0) {
      ratios.push_back(core::kFloatInfinity);
    } else {
      ratio = bounds_column.at(i) / pivot_column.at(i);
      ratios.push_back(ratio);
    }
  }

  // return index of minimum ratio
  auto min_ratio = std::min_element(ratios.begin(), ratios.end());
  int pivot_row_index;
  for (size_t i = 0; i < ratios.size(); ++i) {
    if (std::abs(ratios.at(i) - *min_ratio) < core::kEpsilon) {
      pivot_row_index = i;
    }
  }
  return pivot_row_index;
}

void PrimalSimplex::switchBasis(const int pivot_row_index,
                                const int pivot_column_index) {
  // update basis
  basis_.at(pivot_row_index - 1) = pivot_column_index;
}

void PrimalSimplex::constructNewTable(const int pivot_row_index,
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

bool PrimalSimplex::checkOptimality() {

  const std::vector<float> objective_row = table_.at(0);

  for (size_t i = 0; i < objective_row.size(); ++i) {
    if (objective_row.at(i) < 0) {
      return false;
    }
  }
  return true;
}

void PrimalSimplex::printSolution() {

  // get bounds vector and remove first element
  std::vector<float> bounds = extractColumnFromTable(table_.at(0).size() - 1);
  bounds.erase(bounds.begin());

  // print results to terminal
  for (size_t i = 0; i < bounds.size(); ++i) {
    std::cout << "Variable " << basis_.at(i) << " = " << bounds.at(i)
              << std::endl;
  }
  std::cout << "All other variables were nonbasic and hence = 0" << std::endl;
}

void PrimalSimplex::printTable() {
  for (size_t i = 0; i < table_.size(); ++i) {
    for (size_t j = 0; j < table_.at(0).size(); ++j) {
      std::cout << table_.at(i).at(j) << std::endl;
    }
    std::cout << "  " << std::endl;
  }
}

void PrimalSimplex::printBasis() {
  for (size_t i = 0; i < basis_.size(); ++i) {
    std::cout << basis_.at(i) << std::endl;
  }
  std::cout << " " << std::endl;
}

void PrimalSimplex::solveProblem() {

  for (size_t i = 0; i < core::kMaxIterations; ++i) {
    int pivot_column_index = getPivotColumnIndex();
    int pivot_row_index = getPivotRowIndex(pivot_column_index);
    switchBasis(pivot_row_index, pivot_column_index);
    constructNewTable(pivot_row_index, pivot_column_index);

    if (checkOptimality()) {
      printSolution();
      break;
    }

    if (i == core::kMaxIterations - 1) {
      std::cout << "Max nuber of iterations reached. No solution found."
                << std::endl;
    }
  }
}

} // namespace primal_simplex
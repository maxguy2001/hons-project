#include "bland_simplex.hpp"
#include <algorithm>
#include <cmath>
#include <core/consts.hpp>
#include <iostream>
#include <iterator>

namespace solvers::bland_simplex {

BlandPrimalSimplex::BlandPrimalSimplex() {}

void BlandPrimalSimplex::setProblem(
    const std::vector<std::vector<float>> table) {
  // clear old data
  table_.clear();

  // import new data
  for (size_t i = 0; i < table.size(); ++i) {
    table_.push_back(table.at(i));
  }
}

void BlandPrimalSimplex::setBasis(const std::vector<int> basis) {
  // clear old data
  basis_.clear();

  // import new data
  for (size_t i = 0; i < basis.size(); ++i) {
    basis_.push_back(basis.at(i));
  }
}

std::vector<float>
BlandPrimalSimplex::extractColumnFromTable(const int column_index) {
  std::vector<float> column;
  float table_element;
  for (size_t i = 0; i < table_.size(); ++i) {
    table_element = table_.at(i).at(column_index);
    column.push_back(table_element);
  }
  return column;
}

int BlandPrimalSimplex::getPivotColumnIndex() {

  // get objective function
  const std::vector<float> objective_function = table_.at(0);

  for (std::size_t i = 1; i < objective_function.size(); ++i) {
    if (objective_function.at(i) < 0 &&
        std::find(basis_.begin(), basis_.end(), i) == basis_.end()) {
      return i;
    }
  }
  // if already optimal
  return -1;
}

int BlandPrimalSimplex::getPivotRowIndex(const int pivot_column_index) {

  // extract pivot column
  const std::vector<float> pivot_column =
      extractColumnFromTable(pivot_column_index);

  for (std::size_t i = 1; i < pivot_column.size(); ++i) {
    if (pivot_column.at(i) > 0) {
      return i;
    }
  }
  return -1;
}

bool BlandPrimalSimplex::switchBasis(const int pivot_row_index,
                                     const int pivot_column_index) {

  // check fail state for now
  if (basis_.size() < pivot_row_index) {
    return false;
  }
  // update basis
  basis_.at(pivot_row_index - 1) = pivot_column_index;
  return true;
}

void BlandPrimalSimplex::constructNewTable(const int pivot_row_index,
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

bool BlandPrimalSimplex::checkOptimality() {

  const std::vector<float> objective_row = table_.at(0);

  for (size_t i = 0; i < objective_row.size(); ++i) {
    if (objective_row.at(i) < 0) {
      return false;
    }
  }
  return true;
}

void BlandPrimalSimplex::printObjectiveRow() {
  std::vector<float> obj_row = table_.at(0);
  for (std::size_t i = 0; i < obj_row.size(); ++i) {
    std::cout << obj_row.at(i) << " ";
  }
  std::cout << std::endl;
}

core::SolveStatus
BlandPrimalSimplex::verifySolution(core::InputRows original_problem,
                                   std::vector<float> solution_row) {

  // extract primal solution from solution row
  const int num_primal_variables = original_problem.num_variables - 1;

  std::vector<float> x_negative;
  const int x_neg_upper_bound = solution_row.size() - 1;
  const int x_neg_lower_bound = solution_row.size() - num_primal_variables - 1;
  for (std::size_t i = x_neg_lower_bound; i < x_neg_upper_bound; ++i) {
    x_negative.push_back(solution_row.at(i));
  }

  std::vector<float> x_positive;
  const int x_pos_upper_bound = solution_row.size() - num_primal_variables - 1;
  const int x_pos_lower_bound =
      solution_row.size() - num_primal_variables - num_primal_variables - 1;
  for (std::size_t i = x_pos_lower_bound; i < x_pos_upper_bound; ++i) {
    x_positive.push_back(solution_row.at(i));
  }

  std::vector<float> x;
  for (std::size_t i = 0; i < num_primal_variables; ++i) {
    x.push_back(x_positive.at(i) - x_negative.at(i));
  }

  // check inequalities hold
  float total = 0;
  for (std::size_t i = 0; i < original_problem.inequality_rows.size(); ++i) {
    total += original_problem.inequality_rows.at(i).at(0);
    for (std::size_t j = 0; j < num_primal_variables; ++j) {
      total += original_problem.inequality_rows.at(i).at(j + 1) * x.at(j);
    }
    if (total < 0) {
      return core::SolveStatus::kInfeasible;
    }
    total = 0;
  }

  total = 0;
  for (std::size_t i = 0; i < original_problem.equality_rows.size(); ++i) {
    total += original_problem.equality_rows.at(i).at(0);
    for (std::size_t j = 1; j < num_primal_variables + 1; ++j) {
      total += original_problem.equality_rows.at(i).at(0) * x.at(j - 1);
    }
    if (total != 0) {
      return core::SolveStatus::kInfeasible;
    }
    total = 0;
  }

  return core::SolveStatus::kFeasible;
}

core::SolveStatus
BlandPrimalSimplex::solveProblem(const bool run_verbose,
                                 const core::InputRows original_problem) {

  for (size_t i = 0; i < core::kMaxIterations; ++i) {
    int pivot_column_index = getPivotColumnIndex();
    if (pivot_column_index == -1) {
      ++num_already_optimal_;
      core::SolveStatus solution_status =
          verifySolution(original_problem, table_.at(0));
      return solution_status;
    }
    int pivot_row_index = getPivotRowIndex(pivot_column_index);
    if (pivot_row_index == -1) {
      ++num_pivot_row_failures_;
      return core::SolveStatus::kInfeasible;
    }
    bool is_basis_switch_successful =
        switchBasis(pivot_row_index, pivot_column_index);
    if (!is_basis_switch_successful) {
      ++num_basis_failures_;
      return core::SolveStatus::kError;
    }
    constructNewTable(pivot_row_index, pivot_column_index);

    if (checkOptimality()) {
      core::SolveStatus solution_status =
          verifySolution(original_problem, table_.at(0));
      return solution_status;
    }
  }
  ++num_not_converging_;
  return core::SolveStatus::kDidntConverge;
}

} // namespace solvers::bland_simplex

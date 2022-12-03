#include "presolve.hpp"
#include <iostream>

namespace logical_solver{

  Presolve::Presolve(
    const std::vector<std::vector<int>> problem_matrix, 
    const std::vector<int> lower_bounds, 
    const std::vector<int> upper_bounds,
    const int inequalities_count,
    const int equalities_count
  ) {
    // Define problem
    problem_matrix_ = problem_matrix;
    lower_bounds_ = lower_bounds;
    upper_bounds_ = upper_bounds;

    // Get problem attributes
    variables_count_ = problem_matrix[0].size();
    constraints_count_ = problem_matrix.size();
    inequalities_count_ = inequalities_count;
    equalities_count_ = equalities_count;

    // Set up active rows and columns arrays as well
    // as implied bounds using problem size and feasible
    // solution vector.
    active_rows_.resize(constraints_count_);
    active_columns_.resize(variables_count_);
    implied_lower_bounds_.resize(constraints_count_);
    implied_upper_bounds_.resize(constraints_count_);
    feasible_solution.resize(variables_count_);
    rows_non_zero_variables_.resize(variables_count_);

    std::fill(active_rows_.begin(), active_rows_.end(), true);
    std::fill(active_columns_.begin(), active_columns_.end(), true);
    std::fill(feasible_solution.begin(), feasible_solution.end(), -999);
  };

  std::vector<int> Presolve::getRowNonZeros(int row_index) {
    std::vector<int> non_zero_cols;

    for (int j = 0; j < variables_count_; ++j) {
      if (active_columns_.at(j)) {
        int row_element = problem_matrix_.at(row_index).at(j);

        if (row_element != 0) {
          non_zero_cols.push_back(j);
        }
      }
    }

    return non_zero_cols;
  };

  std::vector<int> Presolve::getColNonZeros(int col_index) {
    std::vector<int> non_zero_rows;

    for (int i = 0; i < constraints_count_; ++i) {
      if (active_rows_.at(i)) {
        int row_element = problem_matrix_.at(i).at(col_index);

        if (row_element != 0) {
          non_zero_rows.push_back(i);
        }
      }
    }

    return non_zero_rows;
  }

  void Presolve::updateStateRowSingletonEquality(
    int row_index, int col_index
  ) {
    int variable_value = lower_bounds_.at(row_index);

    // Set variable equal to its value in the rest of the 
    // rows.
    for (int i = 0; i < constraints_count_; i++) {
      int coefficient = problem_matrix_.at(i).at(col_index);
      problem_matrix_.at(i).at(col_index) = coefficient * variable_value;
    }

    active_rows_.at(row_index) = false;

    // Update presolve stack.
    struct presolve_log log = {row_index, col_index, 1};
    presolve_stack_.push(log);
  };

  void Presolve::applyRowSingletonEqualityPostsolve(
    int row_index, int col_index
  ){
    // This was already found in the update state function for this
    // particular rule, but doing it again to maintain consistency
    // of postsolve functions' functionality.
    int variable_feasible_solution = lower_bounds_.at(row_index);

    feasible_solution.at(col_index) = variable_feasible_solution;
  };

  void Presolve::updateStateEmptyCol(int col_index) {
    active_columns_.at(col_index) = false;

    struct presolve_log log = {-1, col_index, 2};
    presolve_stack_.push(log);
  };

  void Presolve::applyEmptyColPostsolve(int col_index) {
    feasible_solution.at(col_index) = 0;
  };

  void Presolve::applyPresolveRowRules() {
    for (int i = 0; i < constraints_count_; ++i) {
      // If row is active, apply row rules.
      if (active_rows_.at(i)) {
        std::vector<int> row_non_zeros = getRowNonZeros(i);
        int non_zeros_count = row_non_zeros.size();

        // Update non-zero active variables for row.
        rows_non_zero_variables_.at(i) = row_non_zeros;

        // If row is a row singleton, update state accordingly.
        if (non_zeros_count == 1) {
          if (i >= inequalities_count_) {
            // If it is an equality, call row singleton update state method
            // for equalities.
            updateStateRowSingletonEquality(i, row_non_zeros.at(0));
          }
        }
      }
    }
  };

  void Presolve::applyPresolveColRules() {
    for (int j = 0; j < variables_count_; j++) {
      // If column is active, apply col rules.
      if (active_columns_.at(j)) {
        std::vector<int> col_non_zeros = getColNonZeros(j);
        int non_zeros_count = col_non_zeros.size();

        // If column is an empty column, update state accordingly.
        if (non_zeros_count == 0) {
          updateStateEmptyCol(j);
        }
        
        // Check if column is a free column substitution - if only one
        // row is non-zero in the column, and that whole row only
        // has two non-zero variables (including this one.)
        if (non_zeros_count == 1) {
          if (rows_non_zero_variables_.at(col_non_zeros.at(0)).size() == 2) {
            // TODO - add get free col subs get dependency variable funct.
            // add free col subs equality update state, and ineq.
            // 
          }
           
        }
      }
    }
  }

  void Presolve::applyPresolve() {
    applyPresolveRowRules();
    applyPresolveColRules();
  };

  void Presolve::applyPostsolve() {
    while (!presolve_stack_.empty()) {
        presolve_log rule_log = presolve_stack_.top();
        int rule_id = rule_log.rule_id;
        int row_index = rule_log.constraint_index;
        int col_index = rule_log.variable_index;

        if (rule_id == 1) {
          applyRowSingletonEqualityPostsolve(row_index, col_index);
        }
        else if (rule_id == 2) {
          applyEmptyColPostsolve(col_index);
        }
        
        // Remove rule from stack.
        presolve_stack_.pop();
    }
  };

  void Presolve::printFeasibleSolution() {
    for (int i = 0; i < variables_count_; i++) {
      printf("Variable %d = %d\n", i, feasible_solution.at(i));
    }
  };

  void Presolve::printLP() {
    for (int i = 0; i < constraints_count_; ++i) {
      if (active_rows_.at(i)) {
        printf("Row %d \n", i+1);

        for (int j = 0; j < variables_count_; ++j) {
          if (active_columns_.at(j)) {
            std::cout << problem_matrix_[i][j] << std::endl;
          }
        }
        std::cout<<" "<<std::endl;
      }
    }

    std::cout<<" "<<std::endl;
    std::cout<<"Bounds"<<std::endl;;

    for (int i = 0; i < constraints_count_; ++i) {
      std::string upper_bound = std::to_string(upper_bounds_.at(i));
      if (upper_bound == "32765") {
        upper_bound = "Inf";
      }
      std::cout << lower_bounds_.at(i) << ", " << upper_bound << std::endl;
    }
  };
}
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
    // Infitnity - eventually to be defined in types.
    kInfinity = std::numeric_limits<int>::max(); 

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
    active_rows_.resize(constraints_count_, true);
    active_columns_.resize(variables_count_, true);
    implied_lower_bounds_.resize(constraints_count_, -kInfinity);
    implied_upper_bounds_.resize(constraints_count_, kInfinity);
    feasible_solution.resize(variables_count_, -999);

    // Set up 2d vectors to keep track of non-zero variables.
    rows_non_zero_variables_.resize(constraints_count_, {});
    cols_non_zeros_indices_.resize(variables_count_, {});

    active_rows_count_ = constraints_count_;
  };

  void Presolve::getRowsNonZeros() {
    for (int i = 0; i < constraints_count_; i++) {
      std::vector<int> row_non_zero_cols;

      for (int j = 0; j < variables_count_; ++j) {
        if (active_columns_.at(j)) {
          int row_element = problem_matrix_.at(i).at(j);

          if (row_element != 0) {
            row_non_zero_cols.push_back(j);
          }
        }
      }

      rows_non_zero_variables_.at(i) = row_non_zero_cols;
    }
  };

  void Presolve::getColsNonZeros() {
    for (int j = 0; j < variables_count_; j++) {
      std::vector<int> non_zero_coefficients;

      for (int i = 0; i < constraints_count_; ++i) {
        if (active_rows_.at(i)) {
          int col_element = problem_matrix_.at(i).at(j);

          if (col_element != 0) {
            non_zero_coefficients.push_back(i);
          }
        }
      }

      cols_non_zeros_indices_.at(j) = non_zero_coefficients;
    }
  };

  void Presolve::getRowsAndColsNonZeros() {
    for (int i = 0; i < constraints_count_; i++) {
      if (active_rows_.at(i)) {

        for (int j = 0; j < variables_count_; ++j) {
          if (active_columns_.at(j)) {

            if (problem_matrix_.at(i).at(j) != 0) {
              rows_non_zero_variables_.at(i).push_back(j);
              cols_non_zeros_indices_.at(j).push_back(i);
            }
          }
        }
      }
    }
  }

  void Presolve::updateStateRedundantVariable(
    int row_index, int col_index
  ) {
    active_rows_.at(row_index) = false;
    active_columns_.at(col_index) = false;
    active_rows_count_ -= 1;

    // Update presolve stack.
    struct presolve_log log = {row_index, col_index, 1};
    presolve_stack_.push(log);
  };

  void Presolve::updateStateRowSingletonEquality(
    int row_index, int col_index
  ) {
    int variable_value = lower_bounds_.at(row_index);

    // Update the lower bound of each constraint that 
    // contains the variable using the variable value.
    for (auto&i : cols_non_zeros_indices_.at(col_index)) {
      int coefficient = problem_matrix_.at(i).at(col_index);
      int lower_bound = lower_bounds_.at(i);
      lower_bounds_.at(i) = lower_bound - coefficient * variable_value;
    }

    active_rows_.at(row_index) = false;
    active_columns_.at(col_index) = false;
    active_rows_count_ -= 1;

    // Update presolve stack.
    struct presolve_log log = {row_index, col_index, 1};
    presolve_stack_.push(log);
  };

  void Presolve::applyRowSingletonPostsolve(
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

  bool Presolve::isFixedCol(int row_index) {
    if (rows_non_zero_variables_.at(row_index).size() == 2) {
      return true;
    }
    return false;
  };

  int Presolve::getFixedColDependancy(int row_index, int col_index) {
    for (int i = 0; i < 2; i++) {
      int variable_index = rows_non_zero_variables_.at(row_index).at(i);

      if (variable_index != col_index) {
        return variable_index;
      }
    }

    // return -1 if dependancy was not found.
    return -1;
  };

  void Presolve::updateStateFixedCol(int row_index, int col_index) {
    // Get dependancy variable and store in dependancies vector to be 
    // stored in rule log struct.
    int dependancy = getFixedColDependancy(row_index, col_index);
    std::vector<int> dependancies_vector = {dependancy};

    // Turn off row and col
    active_rows_.at(row_index) = false;
    active_columns_.at(col_index) = false;
    active_rows_count_ -= 1;

    // Update presolve stack.
    struct presolve_log log = {row_index, col_index, 3, dependancies_vector};
    presolve_stack_.push(log);
  };

  void Presolve::applyFixedColPostsolve(
    int row_index, int col_index, int dependancy_index
  ) {
    // get variable feasible value.
    int constant_term = lower_bounds_.at(row_index);
    int dependancy_feasible_value = feasible_solution.at(dependancy_index);
    int dependancy_coefficient = problem_matrix_.at(row_index).at(dependancy_index);

    // will need to add a check for when the coefficient is not 1, which may
    // lead to non integer non-feasible values for equalities. In inequalities we can
    // just pick an integer larger then the non-int minimal sol, which will
    // still be feasible if we don't have an implied upper bound affecting 
    // the variable.
    int variable_feasible_value = constant_term - dependancy_feasible_value*dependancy_coefficient;

    feasible_solution.at(col_index) = variable_feasible_value;
    problem_matrix_.at(row_index).at(col_index) = variable_feasible_value;

    active_rows_.at(row_index) = true;
    active_columns_.at(col_index) = true;
  };

  void Presolve::applyPresolveRowRules() {
    for (int i = 0; i < constraints_count_; ++i) {
      // If row is active, apply row rules.
      if (active_rows_.at(i)) {
        std::vector<int> row_non_zeros = rows_non_zero_variables_.at(i);
        int non_zeros_count = row_non_zeros.size();

        // If row is a row singleton, check if it is a redundant 
        // variable, row singleton equality or row singleton inequality.
        if (non_zeros_count == 1) {
          int non_zero_variable = row_non_zeros.at(0);
          int corresponding_col_non_zeros_count = cols_non_zeros_indices_.at(
            non_zero_variable
          ).size();

          // If it is a redundant variable, update state accordingly.
          if (corresponding_col_non_zeros_count == 1) {
            updateStateRedundantVariable(i, non_zero_variable);
          }
          // If it is not a redundant variable, check if is an equality
          // or an inequality and update state accordingly.
          else if (i >= inequalities_count_) {
            // If it is an equality, call row singleton update state method
            // for equalities.
            updateStateRowSingletonEquality(i, non_zero_variable);
          }
        }
      }
    }
  };

  void Presolve::applyPresolveColRules() {
    for (int j = 0; j < variables_count_; j++) {
      // If column is active, apply col rules.
      if (active_columns_.at(j)) {
        int non_zeros_count = cols_non_zeros_indices_.at(j).size();

        // If column is an empty column, update state accordingly.
        if (non_zeros_count == 0) {
          updateStateEmptyCol(j);
        }
        
        // If column is fixed col, update state accordingly.
        if (non_zeros_count == 1) {
          int non_zero_row = cols_non_zeros_indices_.at(j).at(0);

          if (isFixedCol(non_zero_row)) {
            updateStateFixedCol(non_zero_row, j);
          } 
        }
      }
    }
  }

  void Presolve::applyPresolve() {
    while (active_rows_count_ > 0) {
      getRowsAndColsNonZeros();
      applyPresolveRowRules();
      applyPresolveColRules();
    }
  };

  void Presolve::applyPostsolve() {
    while (!presolve_stack_.empty()) {
        presolve_log rule_log = presolve_stack_.top();
        int rule_id = rule_log.rule_id;
        int row_index = rule_log.constraint_index;
        int col_index = rule_log.variable_index;

        if (rule_id == 1) {
          applyRowSingletonPostsolve(row_index, col_index);
        }
        else if (rule_id == 2) {
          applyEmptyColPostsolve(col_index);
        }
        else if (rule_id == 3) {
          applyFixedColPostsolve(row_index, col_index, rule_log.dependancies.at(0));
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
        printf("Row %d \n", i);

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
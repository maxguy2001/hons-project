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
    presolve_active_rows_.resize(constraints_count_, true);
    presolve_active_columns_.resize(variables_count_, true);
    implied_lower_bounds_.resize(constraints_count_, -kInfinity);
    implied_upper_bounds_.resize(constraints_count_, kInfinity);
    feasible_solution.resize(variables_count_, -999);

    presolve_active_rows_count_ = constraints_count_;
    presolve_active_columns_count = variables_count_;
  };

  void Presolve::getRowsNonZeros() {
    for (int i = 0; i < constraints_count_; i++) {
      std::vector<int> row_non_zero_cols;

      for (int j = 0; j < variables_count_; ++j) {
        if (presolve_active_columns_.at(j)) {
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
        if (presolve_active_rows_.at(i)) {
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
    rows_non_zero_variables_.clear();
    cols_non_zeros_indices_.clear();
    rows_non_zero_variables_.resize(constraints_count_, {});
    cols_non_zeros_indices_.resize(variables_count_, {});

    for (int i = 0; i < constraints_count_; i++) {
      if (presolve_active_rows_.at(i)) {

        for (int j = 0; j < variables_count_; ++j) {
          if (presolve_active_columns_.at(j)) {

            if (problem_matrix_.at(i).at(j) != 0) {
              rows_non_zero_variables_.at(i).push_back(j);
              cols_non_zeros_indices_.at(j).push_back(i);
            }
          }
        }
      }
    }
  };

  void Presolve::updateStateRedundantVariable(
    int row_index, int col_index
  ) {
    presolve_active_rows_.at(row_index) = false;
    presolve_active_columns_.at(col_index) = false;
    presolve_active_rows_count_ -= 1;
    presolve_active_columns_count -= 1;

    // Update presolve stack.
    struct presolve_log log = {row_index, col_index, 0};
    presolve_stack_.push(log);
  };

  void Presolve::applyRedundantVariablePostsolve(
    int row_index, int col_index
  ) {
    int feasible_value = lower_bounds_.at(row_index);
    checkVariableFeasibleValue(col_index, feasible_value, 0);
    postsolve_active_rows_.at(row_index) = true;
    postsolve_active_cols_.at(col_index) = true;
  };

  void Presolve::updateStateRowSingletonEquality(
    int row_index, int col_index
  ) {
    int variable_value = lower_bounds_.at(row_index);

    presolve_active_rows_.at(row_index) = false;
    presolve_active_rows_count_ -= 1;
    implied_lower_bounds_.at(col_index) = variable_value;
    implied_upper_bounds_.at(col_index) = variable_value;

    // Update presolve stack.
    struct presolve_log log = {row_index, col_index, 1};
    presolve_stack_.push(log);
  };

  void Presolve::updateStateRowSingletonInequality(
    int row_index, int col_index
  ) {
    // Get the implied bound by divinding the lower bound on the
    // constraint by the coefficient.
    int implied_bound = std::ceil(
      float(lower_bounds_.at(row_index))/float(problem_matrix_.at(row_index).at(col_index))
    );

    if (implied_bound < 0) {
      implied_lower_bounds_.at(row_index) = -implied_upper_bounds_.at(row_index);
      implied_upper_bounds_.at(row_index) = implied_bound;
    } else {
      implied_lower_bounds_.at(row_index) = implied_bound;
    }
  };

  void Presolve::applyRowSingletonPostsolve(int row_index){
    // Since row singleton equality leads to a fixed column 
    // in postsolve the feasible value will already have been
    // found by applyFixedColPostsolve, so we just have to turn 
    // the row on on postsolve so that it can be included in the 
    // constraints checks.
    postsolve_active_rows_.at(row_index) = true;
  };

  void Presolve::updateStateEmptyCol(int col_index) {
    presolve_active_columns_.at(col_index) = false;
    presolve_active_columns_count -= 1;

    struct presolve_log log = {-1, col_index, 2};
    presolve_stack_.push(log);
  };

  void Presolve::applyEmptyColPostsolve(int col_index) {
    feasible_solution.at(col_index) = 0;
    postsolve_active_cols_.at(col_index) = true;
  };

  bool Presolve::isFixedCol(int col_index) {
    if (lower_bounds_.at(col_index) == upper_bounds_.at(col_index)) {
      return true;
    }
    return false;
  };

  void Presolve::updateStateFixedCol(int col_index) {
    int variable_value = implied_lower_bounds_.at(col_index);

    // Update the lower bound of each constraint that 
    // contains the variable using the variable value.
    for (auto&i : cols_non_zeros_indices_.at(col_index)) {
      int coefficient = problem_matrix_.at(i).at(col_index);
      int lower_bound = lower_bounds_.at(i);
      lower_bounds_.at(i) = lower_bound - coefficient * variable_value;
    }

    presolve_active_columns_count -= 1;
    // Log -1 in row index as not applicable in this 
    // rule.
    struct presolve_log log = {-1, col_index, 3};
    presolve_stack_.push(log);
  };

  void Presolve::applyFixedColPostsolve(int col_index) {
    int feasible_value = implied_lower_bounds_.at(col_index);

    // Re-update the lower bound of each constraint that 
    // contains the variable using the variable value.
    for (auto&i : cols_non_zeros_indices_.at(col_index)) {
      int coefficient = problem_matrix_.at(i).at(col_index);
      int lower_bound = lower_bounds_.at(i);
      lower_bounds_.at(i) = lower_bound + coefficient * feasible_value;
    }

    postsolve_active_cols_.at(col_index) = true;
    checkVariableFeasibleValue(col_index, feasible_value, 3);
    feasible_solution.at(col_index) = feasible_value;
  };

  bool Presolve::isFreeColSubstitution(int row_index) {
    if (rows_non_zero_variables_.at(row_index).size() == 2) {
      if (implied_lower_bounds_.at(row_index) == kInfinity && implied_upper_bounds_.at(row_index) == kInfinity) {
        return true;
      }
    }
    return false;
  };

  int Presolve::getFreeColSubstitutionDependancy(
    int row_index, int col_index
  ) {
    for (int i = 0; i < 2; i++) {
      int variable_index = rows_non_zero_variables_.at(row_index).at(i);

      if (variable_index != col_index) {
        return variable_index;
      }
    }

    // return -1 if dependancy was not found.
    return -1;
  };

  void Presolve::updateStateFreeColSubstitution(
    int row_index, int col_index
  ) {
    // Get dependancy variable and store in dependancies vector to be 
    // stored in rule log struct.
    int dependancy = getFreeColSubstitutionDependancy(row_index, col_index);
    std::vector<int> dependancies_vector = {dependancy};

    // Turn off row and col
    presolve_active_rows_.at(row_index) = false;
    presolve_active_columns_.at(col_index) = false;
    presolve_active_rows_count_ -= 1;
    presolve_active_columns_count -= 1;

    // Update presolve stack.
    struct presolve_log log = {row_index, col_index, 4, dependancies_vector};
    presolve_stack_.push(log);
  };

  void Presolve::applyFreeColSubstitutionPostsolve(
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
    int feasible_value = constant_term - dependancy_feasible_value*dependancy_coefficient;

    checkVariableFeasibleValue(col_index, feasible_value, 4);
    feasible_solution.at(col_index) = feasible_value;

    postsolve_active_rows_.at(row_index) = true;
    postsolve_active_cols_.at(col_index) = true;
  };

  void Presolve::applyPresolveRowRules() {
    for (int i = 0; i < constraints_count_; ++i) {
      // If row is active, apply row rules.
      if (presolve_active_rows_.at(i)) {
        int non_zeros_count = rows_non_zero_variables_.at(i).size();

        // If row is a row singleton, check if it is a redundant 
        // variable, row singleton equality or row singleton inequality.
        if (non_zeros_count == 1) {
          int non_zero_variable = rows_non_zero_variables_.at(i).at(0);
          int corresponding_col_non_zeros_count = cols_non_zeros_indices_.at(
            non_zero_variable
          ).size();

          // If it is a redundant variable, update state accordingly.
          if (corresponding_col_non_zeros_count == 1) {
            updateStateRedundantVariable(i, non_zero_variable);
          }
          // If it is not a redundant variable, check if is an equality
          // or an inequality and update state accordingly.
          else if (i < inequalities_count_) {
            updateStateRowSingletonInequality(i, non_zero_variable);
          }
          else {
            updateStateRowSingletonEquality(i, non_zero_variable);
          }
        }
      }
    }
  };

  void Presolve::applyPresolveColRules() {
    for (int j = 0; j < variables_count_; j++) {
      // If column is active, apply col rules.
      if (presolve_active_columns_.at(j)) {
        int non_zeros_count = cols_non_zeros_indices_.at(j).size();
        
        // If column is a fixed column update state 
        // accordingly.
        if (isFixedCol(j)) {
          updateStateFixedCol(j);
        }
        // If column is an empty column, update state accordingly.
        else if (non_zeros_count == 0) {
          updateStateEmptyCol(j);
        }
        // If column is fixed col, update state accordingly.
        else if (non_zeros_count == 1) {
          int non_zero_row = cols_non_zeros_indices_.at(j).at(0);

          if (isFreeColSubstitution(non_zero_row)) {
            updateStateFreeColSubstitution(non_zero_row, j);
          } 
        }
      }
    }
  };

  void Presolve::checkVariableFeasibleValue(
    int col_index, int feasible_value, int rule_id
  ) {
    try {
      if (feasible_value < implied_lower_bounds_.at(col_index) || feasible_value > implied_upper_bounds_.at(col_index)) {
        throw(0);
      }
    }
    catch (int myNum) {
      printf("Variable %d does not meet the implied bounds.", col_index);
      printf("Implied bounds: Lower = %d, Upper = %d.", implied_lower_bounds_.at(col_index), implied_upper_bounds_.at(col_index));
      printf("Rule used: %d", rule_id);
    }
  };

  std::vector<int> Presolve::getUnsatisfiedConstraintsPostsolve() {
    std::vector<int> unsatisfied_constraints = {};

    for (int i = 0; i < constraints_count_; i++) {
      if (postsolve_active_rows_.at(i)) {
        int constraint_value = 0;

        // Loop through row active columns working out the 
        // constraint value.
        for (int j = 0; j < variables_count_; j++) {
          if (postsolve_active_cols_.at(j)) {
            constraint_value += problem_matrix_.at(i).at(j)*feasible_solution.at(j);
          }
        }

        // Check if constraint is satisfied, and if not return 
        // constraint index.
        if (constraint_value < lower_bounds_.at(i) || constraint_value > upper_bounds_.at(i)) {
          unsatisfied_constraints.push_back(i);
        }
      }
    }
  
    //If all constraints were satisfied, return -1.
    return unsatisfied_constraints;
  };

  void Presolve::checkUnsatisfiedConstraintsPostsolve(
    std::vector<int> unsatisfied_constraints,
    int rule_id
  ) {
    try {
      if (unsatisfied_constraints.size() > 0) {
        throw (0);
      }
    }   
    catch (int myNum) {
      std::cout<<"The following constraints were unsatisfied:"<<std::endl;
      for (int i = 0; i < unsatisfied_constraints.size(); i++) {
        std::cout<<i<<std::endl;
      }
      printf("After applying rule %d", rule_id);
    }
  };

  void Presolve::applyPresolve() {
    int iteration_active_rows = presolve_active_rows_count_;
    int iteration_active_cols = presolve_active_columns_count;

    while (presolve_active_rows_count_ > 0) {
      getRowsAndColsNonZeros();
      applyPresolveRowRules();
      applyPresolveColRules();

      if (presolve_active_rows_count_ == iteration_active_rows && presolve_active_columns_count == iteration_active_cols) {
        break;
      } else {
        iteration_active_rows = presolve_active_rows_count_;
        iteration_active_cols = presolve_active_columns_count;
      }
    }
  };

  void Presolve::applyPostsolve() {
    postsolve_active_rows_.resize(constraints_count_, false);
    postsolve_active_cols_.resize(variables_count_, false);
    
    while (!presolve_stack_.empty()) {
      presolve_log rule_log = presolve_stack_.top();
      int rule_id = rule_log.rule_id;
      int row_index = rule_log.constraint_index;
      int col_index = rule_log.variable_index;

      if (rule_id == 0) {
        applyRedundantVariablePostsolve(row_index, col_index);
      }
      else if (rule_id == 1) {
        applyRowSingletonPostsolve(row_index);
      }
      else if (rule_id == 2) {
        applyEmptyColPostsolve(col_index);
      }
      else if (rule_id == 3) {
        applyFixedColPostsolve(col_index);
      }
      else if (rule_id == 4) {
        applyFreeColSubstitutionPostsolve(row_index, col_index, rule_log.dependancies.at(0));
      }
      
      // Remove rule from stack.
      presolve_stack_.pop();
      std::vector<int> unsatisfied_constraints = getUnsatisfiedConstraintsPostsolve();
      // checkUnsatisfiedConstraintsPostsolve(unsatisfied_constraints,rule_id);
    }
  };

  void Presolve::printFeasibleSolution() {
    for (int i = 0; i < variables_count_; i++) {
      printf("Variable %d = %d\n", i, feasible_solution.at(i));
    }
  };

  void Presolve::printLP() {
    for (int i = 0; i < constraints_count_; ++i) {
      if (presolve_active_rows_.at(i)) {
        printf("Row %d \n", i);

        for (int j = 0; j < variables_count_; ++j) {
          if (presolve_active_columns_.at(j)) {
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
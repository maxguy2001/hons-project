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
    inequality_singletons_ = {};

    implied_lower_bounds_.resize(variables_count_, -kInfinity);
    implied_upper_bounds_.resize(variables_count_, kInfinity);
    feasible_solution.resize(variables_count_, -999);

    reduced_to_empty = false;
    infeasible = false;

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

  int Presolve::getVariableFeasibleValue(
    int row_index, int col_index, 
    int variable_coefficient, int constraint_RHS
  ) {
    // Check if RHS/coeff is not an integer
    if (constraint_RHS % variable_coefficient != 0) {
      // If inequality, check if rounding up satisfies upper
      // bound and implied bounds.
      if (row_index < inequalities_count_) {
        int feasible_value = ceil(constraint_RHS/variable_coefficient);
        if (feasible_value <= upper_bounds_.at(row_index)) {
          if (checkVariableImpliedBounds(col_index, feasible_value)) {
            return feasible_value;
          }
        }
      }
      return kInfinity;
    }
    // If RHS/coeff is integer, calculate it and check if 
    // implied bounds satisfied.
    int feasible_value = constraint_RHS/variable_coefficient;
    if (checkVariableImpliedBounds(col_index, feasible_value)) {
      return feasible_value;
    }
    return kInfinity;
  };

  bool Presolve::updateStateFeasibleValue(int col_index, int feasible_value) {
    if (feasible_value == kInfinity) {
      infeasible = true;
      return false;
    }
    feasible_solution.at(col_index) = feasible_value;
    return true;
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
    int feasible_value = getVariableFeasibleValue(
      row_index, col_index, problem_matrix_.at(row_index).at(col_index), 
      lower_bounds_.at(row_index)
    );
    if (updateStateFeasibleValue(col_index, feasible_value)) {
      feasible_solution.at(col_index) = feasible_value;
      postsolve_active_rows_.at(row_index) = true;
      postsolve_active_cols_.at(col_index) = true;
    }
  };

  void Presolve::updateStateRowSingletonEquality(
    int row_index, int col_index
  ) {
    int variable_coefficient = problem_matrix_.at(row_index).at(col_index);
    int RHS = lower_bounds_.at(row_index);

    if (RHS %  variable_coefficient == 0) {
      int variable_value = RHS/variable_coefficient;
      presolve_active_rows_.at(row_index) = false;
      presolve_active_rows_count_ -= 1;
      implied_lower_bounds_.at(col_index) = variable_value;
      implied_upper_bounds_.at(col_index) = variable_value;

      // Update presolve stack.
      struct presolve_log log = {row_index, col_index, 1};
      presolve_stack_.push(log);
    } else {
      infeasible = true;
    }
  };

  void Presolve::updateStateRowSingletonInequality(
    int row_index, int col_index
  ) {
    // Get the implied bound by divinding the lower bound on the
    // constraint by the coefficient.
    int variable_coefficient = problem_matrix_.at(row_index).at(col_index);

    if (variable_coefficient < 0) {
      // If coefficient negative, upper bound becomes 
      // lower bound divided by coefficient, and we use floor
      // to make sure that the bound is met in integer arithmetic.
      int implied_bound = std::floor(
        float(lower_bounds_.at(row_index))/float(variable_coefficient)
      );
      implied_lower_bounds_.at(col_index) = -implied_upper_bounds_.at(col_index);
      implied_upper_bounds_.at(col_index) = implied_bound;

    } else {
      // If coefficient not negative, lower bound becomes lower
      // bound divided by the coefficient and we use ceil to make
      // bound is met in integer arithmetic.
      int implied_bound = std::ceil(
        float(lower_bounds_.at(row_index))/float(variable_coefficient)
      );
      implied_lower_bounds_.at(col_index) = implied_bound;
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

  bool Presolve::checkAreRowsParallel(int row1_index, int row2_index) {
    if (rows_non_zero_variables_.at(row1_index).size() != rows_non_zero_variables_.at(row2_index).size()) {
      return false;
    }
    int row1_first_non_zero_col = rows_non_zero_variables_.at(row1_index).at(0);
    int row2_first_non_zero_col = rows_non_zero_variables_.at(row2_index).at(0);
    if (row1_first_non_zero_col != row2_first_non_zero_col) {
      return false;
    }
    double ratio = problem_matrix_.at(row1_index).at(row1_first_non_zero_col)/problem_matrix_.at(row2_index).at(row2_first_non_zero_col);

    for (int j = 1; j < rows_non_zero_variables_.at(row1_index).size(); j++) {
      int row1_non_zero_col = rows_non_zero_variables_.at(row1_index).at(j);
      int row2_non_zero_col = rows_non_zero_variables_.at(row2_index).at(j);

      if (row1_non_zero_col != row2_non_zero_col) {
        return false;
      }
      double ratio_new = problem_matrix_.at(row1_index).at(row1_non_zero_col)/problem_matrix_.at(row2_index).at(row2_non_zero_col);
      if (ratio != ratio_new) {return false;}
      ratio = ratio_new;
    }

    return true;
  };

  int Presolve::getParallelRow(int row_index, int start) {
    if (row_index == start) {return -1;}
    
    for (int k = start; k < row_index; k++) {
      if (checkAreRowsParallel(row_index, k)) {return k;}
    }

    return -1;
  };

  std::vector<int> Presolve::sortParallelRowsBySize(int row, int parallel_row) {
    // Get index of first non-zero variable in row.
    int first_non_zero_col = rows_non_zero_variables_.at(row).at(0);

    // Get absolute value of first coefficient of each row.
    int row_abs_first_coeff = std::abs(problem_matrix_.at(row).at(first_non_zero_col));
    int parallel_row_abs_first_coeff = std::abs(problem_matrix_.at(parallel_row).at(first_non_zero_col));
    
    if (row_abs_first_coeff > parallel_row_abs_first_coeff) {
      return {parallel_row, row};
    }
    // If they are equal then return parallel row so that
    // row is kept on, which means we might still detect if it
    // is a row singleton or reduntant variable afterwards, which would
    // mean we turn off both rows in the same iteration.
    return {row, parallel_row};
  };

  bool Presolve::checkAreParallelRowsFeasible(
    int small_row_index,
    int large_to_small_ratio,
    double large_bound_by_ratio
  ){
    // If we are dealing with an equality, check if the bounds
    // are also multiples of eachother and if not the parallel
    // row is not feasible.
    if (small_row_index >= inequalities_count_) {
      if (large_bound_by_ratio != lower_bounds_.at(small_row_index)) {
        return false;
      }
    }
    // If we have one row of each sign, we now have to check feasibility
    // for inequalities.
    if (large_to_small_ratio < 0) {
      // large bound by ratio, rounded down to ensure feasibility for
      // integers, now becomes an upper bound of the small inequality.
      // Thus, if it is smaller than the small row's bound, the problem 
      // is infeasible.
      if (std::floor(large_bound_by_ratio) < lower_bounds_.at(small_row_index)) {
        return false;
      }
    }

    return true;

  };

  void Presolve::updateStateParallelRow(
    int small_row_index, 
    int large_row_index,
    int large_to_small_ratio,
    double large_bound_by_ratio
  ) {
    // turn off large row and log into stack
    presolve_active_rows_.at(large_row_index) = false;
    presolve_active_rows_count_ -= 1;

    // Dependancy vector to keep track of the row we keep on and its
    // original bound.
    std::vector<int> dependencies = {small_row_index, lower_bounds_.at(small_row_index)};
    struct presolve_log log = {large_row_index, -1, 5, dependencies};
    presolve_stack_.push(log);

    // if we have an inequality, update the bound on the small row 
    // to ensure that both are satisfied.
    if (small_row_index < inequalities_count_) {
      // Round up the large bound by ratio to ensure conditions
      // are met integer wise.
      int rounded_large_bound_by_ratio = std::ceil(large_bound_by_ratio);
      int small_row_lower_bound = lower_bounds_.at(small_row_index);

      if (large_to_small_ratio > 0) {
        // If the inequalities both have the same sign then we use 
        // the bound that takes up all the slack.
        if (small_row_lower_bound < rounded_large_bound_by_ratio) {
          lower_bounds_.at(small_row_index) = rounded_large_bound_by_ratio;
        }
      } else {
        // If they do not have the same sign, then that means that the 
        // lower bound of the large one divided by the large 
        // to small ratio, rounded down, will have become an upper bound
        // of the small one - if this upper bound is smaller than the small one's
        // lower bound, we will have already deemed the system unfeasible in 
        // checkAreParallelRowsFeasible.
        upper_bounds_.at(small_row_index) = std::floor(large_bound_by_ratio);
      }
    }
  };

  void Presolve::applyParallelRowPostsolve(
    int row_index, int small_row_index, int small_row_initial_bound
  ) {
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
    if (implied_lower_bounds_.at(col_index) == implied_upper_bounds_.at(col_index)) {
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

    presolve_active_columns_.at(col_index) = false;
    presolve_active_columns_count -= 1;
    // Log -1 in row index as not applicable in this 
    // rule.
    struct presolve_log log = {-1, col_index, 3};
    presolve_stack_.push(log);
  };

  void Presolve::applyFixedColPostsolve(int col_index) {
    // In postsolve we know that the feasible value 
    // is feasible because when updating the implied 
    // bounds in presolve we check that the implied 
    // bounds we get are integers.
    int feasible_value = implied_lower_bounds_.at(col_index);

    // Re-update the lower bound of each constraint that 
    // contains the variable using the variable value.
    for (auto&i : cols_non_zeros_indices_.at(col_index)) {
      int coefficient = problem_matrix_.at(i).at(col_index);
      int lower_bound = lower_bounds_.at(i);
      lower_bounds_.at(i) = lower_bound + coefficient * feasible_value;
    }

    postsolve_active_cols_.at(col_index) = true;
    feasible_solution.at(col_index) = feasible_value;
  };

  bool Presolve::isFreeColSubstitution(int row_index, int col_index) {
    if (rows_non_zero_variables_.at(row_index).size() == 2) {
      if (implied_lower_bounds_.at(col_index) == -kInfinity && implied_upper_bounds_.at(col_index) == kInfinity) {
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
    // Check if a feasible value has been found for a 
    // variable, and if so apply postsolve.
    if (postsolve_active_cols_.at(dependancy_index)) {
      int constant_term = lower_bounds_.at(row_index);
      int dependancy_feasible_value = feasible_solution.at(dependancy_index);
      int dependancy_coefficient = problem_matrix_.at(row_index).at(dependancy_index);
      int RHS = constant_term - dependancy_feasible_value*dependancy_coefficient;
      int variable_coefficient = problem_matrix_.at(row_index).at(col_index);


      int feasible_value = getVariableFeasibleValue(
        row_index, col_index, variable_coefficient, RHS
      );
      if (updateStateFeasibleValue(col_index, feasible_value)) {
        postsolve_active_rows_.at(row_index) = true;
        postsolve_active_cols_.at(col_index) = true;
      }
    }
  };

  void Presolve::applyPresolveRowRules() {
    for (int i = 0; i < constraints_count_; ++i) {
      // If row is active, apply row rules.
      if (presolve_active_rows_.at(i)) {
        // Check if row is parallel to another row from row 0
        // to row i-1, or from the start of the equalities to i-1.
        int parallel_row_search_start = 0;
        if (i >= inequalities_count_) {parallel_row_search_start = inequalities_count_;}
        // std::cout << "TRYYYY" <<std::endl;
        // std::cout << i << std::endl;
        
        int parallel_row = getParallelRow(i, parallel_row_search_start);
        // If parallel row is found, check feasibility and 
        // if feasible call updateSateParallelRows.

        if (parallel_row != -1) {
          std::vector<int> sorted_rows = sortParallelRowsBySize(i, parallel_row);
          int small_row_index = sorted_rows.at(0);
          int large_row_index = sorted_rows.at(1);
          int large_to_small_ratio = problem_matrix_.at(large_row_index).at(rows_non_zero_variables_.at(large_row_index).at(0))/problem_matrix_.at(small_row_index).at(rows_non_zero_variables_.at(small_row_index).at(0));
          double large_bound_by_ratio;
          // std::cout << "TRYYYY 3" <<std::endl;
          // if (i == 5) {
          //   printf("parallel row: %d\n", parallel_row);
          //   printf("ratio: %d\n", large_to_small_ratio);
          //   printf("large lower bound: %d\n", lower_bounds_.at(large_row_index));
          // }
          if (lower_bounds_.at(large_row_index) == -2147483648) {
            large_bound_by_ratio = -2147483648/large_to_small_ratio;
          } else {
            large_bound_by_ratio = lower_bounds_.at(large_row_index)/large_to_small_ratio;
          }

          // std::cout << "TRYYYY 4" <<std::endl;
          // If parallel row not feasible, set problem to infeasible
          // and break, else update state.
          if (!checkAreParallelRowsFeasible(small_row_index, large_to_small_ratio, large_bound_by_ratio)) {
            infeasible = true;
            break;
          } else {
            updateStateParallelRow(
              small_row_index, 
              large_row_index, 
              large_to_small_ratio,
              large_bound_by_ratio
            );
          }
          // if I was the large row in parallel rows it will have 
          // been turned off so we don't check the rest of the rules.
          if (large_row_index == i) {continue;}
        }
        int non_zeros_count = rows_non_zero_variables_.at(i).size();
        // If row is a row singleton, check if it is a redundant 
        // variable, row singleton equality or row singleton inequality,
        // and update state accordingly.
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
          else if (i < inequalities_count_) { // Inequality
            if (std::find(inequality_singletons_.begin(), inequality_singletons_.end(), i) == inequality_singletons_.end()) {
              updateStateRowSingletonInequality(i, non_zero_variable);
              inequality_singletons_.push_back(i);
            }
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
        // If column is free col, update state accordingly.
        else if (non_zeros_count == 1) {
          int non_zero_row = cols_non_zeros_indices_.at(j).at(0);

          if (isFreeColSubstitution(non_zero_row, j)) {
            updateStateFreeColSubstitution(non_zero_row, j);
          } 
        }
      }
    }
  };

  bool Presolve::checkVariableImpliedBounds(
    int col_index, int feasible_value
  ) {
    if (feasible_value < implied_lower_bounds_.at(col_index) || feasible_value > implied_upper_bounds_.at(col_index)) {
      printf("Variable %d does not meet the implied bounds.", col_index);
      printf("Implied bounds: Lower = %d, Upper = %d.", implied_lower_bounds_.at(col_index), implied_upper_bounds_.at(col_index));
      return false;
    }
    return true;
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

        // Check if constraint is satisfied, and if not 
        // add index to unsatisfied constraints vector.
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
    if (unsatisfied_constraints.size() > 0) {
      std::cout<<"The following constraints were unsatisfied:"<<std::endl;
      for (int i = 0; i < unsatisfied_constraints.size(); i++) {
        std::cout<<unsatisfied_constraints.at(i)<<std::endl;
      }
      printf("After applying rule %d \n", rule_id);
      std::cout << " " << std::endl;
    }
  };

  void Presolve::applyPresolve() {
    int iteration_active_rows = presolve_active_rows_count_;
    int iteration_active_cols = presolve_active_columns_count;
    while (presolve_active_rows_count_ > 0) {
      getRowsAndColsNonZeros();
      applyPresolveRowRules();
      applyPresolveColRules();
      if (infeasible) {break;}
      
      if (presolve_active_rows_count_ == iteration_active_rows && presolve_active_columns_count == iteration_active_cols) {
        break;
      } else {
        iteration_active_rows = presolve_active_rows_count_;
        iteration_active_cols = presolve_active_columns_count;
      }
    }

    if (presolve_active_rows_count_ == 0 && presolve_active_columns_count == 0) {
      reduced_to_empty = true;
    }
  };

  void Presolve::applyPostsolve() {
    postsolve_active_rows_.resize(constraints_count_, false);
    postsolve_active_cols_.resize(variables_count_, false);
    if (!infeasible) {
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

        if (infeasible) {break;}
        // Remove rule from stack.
        presolve_stack_.pop();
        std::vector<int> unsatisfied_constraints = getUnsatisfiedConstraintsPostsolve();
        checkUnsatisfiedConstraintsPostsolve(unsatisfied_constraints, rule_id);
      }
    }
  };

  void Presolve::printFeasibleSolution() {
    for (int i = 0; i < variables_count_; i++) {
      printf("Variable %d = %d\n", i, feasible_solution.at(i));
    }
    std::cout<<" "<<std::endl;
  };

  void Presolve::printBounds() {
    std::cout<<"Constraints Bounds"<<std::endl;;

    for (int i = 0; i < constraints_count_; ++i) {
      std::string upper_bound = std::to_string(upper_bounds_.at(i));
      if (upper_bound == "32765") {
        upper_bound = "Inf";
      }
      std::cout << lower_bounds_.at(i) << ", " << upper_bound << std::endl;
    }

    std::cout<<" "<<std::endl;
  };

  void Presolve::printImpliedBounds() {
    std::cout<<"Implied Bounds"<<std::endl;;

    for (int i = 0; i < variables_count_; ++i) {
      int lower_bound = implied_lower_bounds_.at(i);
      int upper_bound = implied_upper_bounds_.at(i);

      std::string lower_str;
      std::string upper_str;

      if (lower_bound == -kInfinity) {
        lower_str = "-Inf";
      } else {
        lower_str = std::to_string(lower_bound);
      }

      if (upper_bound == kInfinity) {
        upper_str = "Inf";
      } else {
        upper_str = std::to_string(upper_bound);
      }
      
      std::cout << lower_str << ", " << upper_str << std::endl;
    }

    std::cout<<" "<<std::endl;
  };

  void Presolve::printLP() {
    int max_width = 0;
    for (const auto &row : problem_matrix_) {
      for (int element : row) {
        int width = std::to_string(element).length();
        max_width = std::max(max_width, width);
      }
    }

    for (const auto &row : problem_matrix_) {
      for (int element : row) {
        std::cout << std::left << std::setw(max_width) << element << " ";
      }
      std::cout << std::endl;
    }
    
    std::cout<<" "<<std::endl;

    printBounds();
  };

  void Presolve::printPresolveCurrentState() {
    for (int i=0; i < constraints_count_; i++) {
      if (rows_non_zero_variables_.at(i).size() > 0) {
        printf("ROW %d\n", i);
        for (int j = 0; j < rows_non_zero_variables_.at(i).size(); j++) {
          printf("Col %d: %d\n", j, problem_matrix_.at(i).at(j));
        }
        std::cout<<""<<std::endl;
      }
    }
  }
}
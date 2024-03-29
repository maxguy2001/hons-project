#include "presolve.hpp"
#include <iostream>

namespace logical_solver{

  Presolve::Presolve(
    const std::vector<std::vector<int>> problem_matrix, 
    const std::vector<int> lower_bounds, 
    const std::vector<int> upper_bounds,
    const int inequalities_count,
    const int equalities_count,
    const bool solve_ip
    ) 
    :problem_matrix_(problem_matrix),
    solve_ip_(solve_ip),
    variables_count_(problem_matrix[0].size()),
    constraints_count_(problem_matrix.size()),
    inequalities_count_(inequalities_count),
    equalities_count_(equalities_count),
    reduced_to_empty_(false),
    infeasible_(false),
    infeasible_by_PR_(false),
    unsatisfied_constraints_(false),
    print_unsatisfied_constraints_(false),
    presolve_active_rows_count_(constraints_count_),
    presolve_active_cols_count_(variables_count_)
  {
    // Set up constraints lower and upper bounds as 
    // vector of doubles.
    for (std::size_t i=0; i < constraints_count_; ++i) {
      if (lower_bounds.at(i) == -2147483648) {
        lower_bounds_.push_back(static_cast<double>(-core::kIntInfinity));
      } else {
        lower_bounds_.push_back(static_cast<double>(lower_bounds.at(i)));
      }
      upper_bounds_.push_back(static_cast<double>(upper_bounds.at(i)));
    }

    // Set up active rows and columns arrays as well
    // as implied bounds using problem size and feasible
    // solution vector.
    presolve_active_rows_.resize(constraints_count_, true);
    presolve_active_columns_.resize(variables_count_, true);
    inequality_singletons_ = {};

    implied_lower_bounds_.resize(variables_count_, -core::kIntInfinity);
    implied_upper_bounds_.resize(variables_count_, core::kIntInfinity);
    feasible_solution_.resize(variables_count_, -999);
  }

  void Presolve::getRowsAndColsNonZeros() {
    rows_non_zero_variables_.clear();
    cols_non_zeros_indices_.clear();
    rows_non_zero_variables_.resize(constraints_count_, {});
    cols_non_zeros_indices_.resize(variables_count_, {});

    for (std::size_t i = 0; i < constraints_count_; ++i) {
      if (presolve_active_rows_.at(i)) {

        for (std::size_t j = 0; j < variables_count_; ++j) {
          if (presolve_active_columns_.at(j)) {

            if (problem_matrix_.at(i).at(j) != 0) {
              rows_non_zero_variables_.at(i).push_back(j);
              cols_non_zeros_indices_.at(j).push_back(i);
            }
          }
        }
      }
    }
  }

  double Presolve::getFeasibleValueCalculationBound(const int row_index) {
    double lower_bound = lower_bounds_.at(row_index);
    double upper_bound = upper_bounds_.at(row_index);

    if (lower_bound != -core::kIntInfinity) {
      return lower_bound;
    }
    return upper_bound;
  }

  int Presolve::getDependancyIndexRowDoubletonIP(
    const int row_index, const int col_index
  ) {
    int nonzero_count = 0;
    int dependancy_index = -1;

    for (std::size_t j = 0; j < variables_count_; ++j) {
      int coefficient = problem_matrix_.at(row_index).at(j);
      if (coefficient != 0) {
        if (nonzero_count == 0 && j != col_index) {
          dependancy_index = j;
          nonzero_count += 1;
          return dependancy_index;
        }
        else if (nonzero_count == 1) {
          return -1;
        }
      }
    }

    return dependancy_index;
  }

  bool Presolve::checkDependancyIP(
    const int dependancy_new_feasible_value, 
    const int dependancy_col_index
  ) {
    double dependancy_feasible_value = feasible_solution_.at(dependancy_col_index);

    for (std::size_t i; i < constraints_count_; ++i) {
      if (problem_matrix_.at(i).at(dependancy_col_index) != 0 && postsolve_active_rows_.at(i)) {
        feasible_solution_.at(dependancy_col_index) = dependancy_feasible_value;
        bool is_constraint_satisfied = checkConstraint(i, 0);
        feasible_solution_.at(dependancy_col_index) = dependancy_feasible_value;

        if (!is_constraint_satisfied) {return false;}
      }
    }

    return true;
  }

  int Presolve::getVariableFeasibleValueIP(
    const int row_index, const int col_index, 
    const int variable_coefficient, const double constraint_RHS
  ) {
    // Check if RHS/coeff is not an integer
    if (static_cast<int>(constraint_RHS) % variable_coefficient != 0) {
      // If inequality, check if rounding satisfies implied bounds.
      if (row_index < inequalities_count_) {
        int feasible_value;

        if (variable_coefficient > 0) {
          feasible_value = ceil(constraint_RHS/variable_coefficient);
        } else {
          feasible_value = floor(constraint_RHS/variable_coefficient);
        }

        if (checkVariableImpliedBounds(col_index, feasible_value)) {
          return feasible_value;
        }
      } 
      // We now know we have an equality which is not satisfied.
      // We will check if there is only one other variable in the row,
      // and if so we will attempt to change the feasible value
      // of the other variable to make the right hand side of the equality
      // equal 1.
      int dependancy_index = getDependancyIndexRowDoubletonIP(
        row_index, col_index
      );
      if (dependancy_index != -1) {
        double dependancy_potential_value = (variable_coefficient - lower_bounds_.at(row_index))/problem_matrix_.at(row_index).at(dependancy_index);
        if (dependancy_potential_value == floor(dependancy_potential_value)) {
          if (checkDependancyIP(dependancy_potential_value, dependancy_index)) {
            feasible_solution_.at(dependancy_index) = dependancy_potential_value;
            return 1;
          }
        }
      }
      return core::kIntInfinity;
    }
    // If RHS/coeff is integer, calculate it and check if 
    // implied bounds satisfied.
    int feasible_value = constraint_RHS/variable_coefficient;
    if (checkVariableImpliedBounds(col_index, feasible_value)) {
      return feasible_value;
    }
    return core::kIntInfinity;
  }

  bool Presolve::isFeasibleSolutionInteger() {
    for (const double& feasible_value : feasible_solution_) {
        if (std::floor(feasible_value) != feasible_value) {
            // feasible value is not an integer.
            return false;
        }
    }
    return true;
  }

  bool Presolve::checkIsRowFree(const int row_index) {
    if (solve_ip_) {
      if (lower_bounds_.at(row_index) == -2147483646.000000) {
        lower_bounds_.at(row_index) = -core::kIntInfinity;
      }
    }
    if (lower_bounds_.at(row_index) == -core::kIntInfinity && upper_bounds_.at(row_index) == core::kIntInfinity) {
      return true;
    }
    return false;
  }

  void Presolve::updateStateFreeRow(const int row_index) {
    presolve_active_rows_.at(row_index) = false;
    presolve_active_rows_count_ -= 1;

    // Update presolve stack.
    struct presolve_log log = {
      row_index, -1, 
      static_cast<int>(core::PresolveRulesIds::freeRowId)
    };
    presolve_stack_.push(log);
  }

  void Presolve::applyFreeRowPostsolve(
    const int row_index
  ) {
    postsolve_active_rows_.at(row_index) = true;
  }

  void Presolve::updateStateRowAndColSingleton(
    const int row_index, const int col_index
  ) {
    presolve_active_rows_.at(row_index) = false;
    presolve_active_columns_.at(col_index) = false;
    presolve_active_rows_count_ -= 1;
    presolve_active_cols_count_ -= 1;

    // Update presolve stack.
    struct presolve_log log = {
      row_index, col_index, 
      static_cast<int>(core::PresolveRulesIds::rowAndColSingletonId)
    };
    presolve_stack_.push(log);
  }

  void Presolve::applyRowAndColSingletonPostsolve(
    const int row_index, const int col_index
  ) {
    int variable_coeff = problem_matrix_.at(row_index).at(col_index);
    double feasibleValueCalculationBound = getFeasibleValueCalculationBound(
      row_index
    );
    double feasible_value;

    if (solve_ip_) {
      feasible_value = getVariableFeasibleValueIP(
        row_index, col_index, problem_matrix_.at(row_index).at(col_index), 
        feasibleValueCalculationBound
      );
    } else {
      feasible_value = feasibleValueCalculationBound/variable_coeff;
    }
    if (feasible_value == core::kIntInfinity) {infeasible_ = true;}
    else {
      feasible_solution_.at(col_index) = feasible_value;
      postsolve_active_cols_.at(col_index) = true;
      // Check if we have found feasible values for all variables in the 
      // row. If so, check if the row (constraint) is satisfied, and if 
      // so turn on row in postsolve.
      if (isRowActivePostsolve(row_index)) {
        if (checkConstraint(row_index, static_cast<int>(core::PresolveRulesIds::rowAndColSingletonId))) {
          postsolve_active_rows_.at(row_index) = true;
        }
      }
    }
  }

  void Presolve::updateStateRowSingletonEquality(
    const int row_index, const int col_index
  ) {
    int variable_coefficient = problem_matrix_.at(row_index).at(col_index);
    int RHS = lower_bounds_.at(row_index);
    // In row singleton equality, if we are solving the ip,
    // we check it is feasible in presolve already so that we 
    // do not continue the process of it is not feasible.
    double variable_value;
    if (solve_ip_) {
      variable_value = getVariableFeasibleValueIP(
        row_index, col_index, variable_coefficient, RHS
      );
    } else {
      variable_value = static_cast<double>(RHS)/variable_coefficient;
    } 
    if (variable_value != core::kIntInfinity) {
      presolve_active_rows_.at(row_index) = false;
      presolve_active_rows_count_ -= 1;
      implied_lower_bounds_.at(col_index) = variable_value;
      implied_upper_bounds_.at(col_index) = variable_value;

      // Update presolve stack.
      struct presolve_log log = {
        row_index, col_index, 
        static_cast<int>(core::PresolveRulesIds::rowSingletonId)
      };
      presolve_stack_.push(log);
    } else {
      infeasible_ = true;
    }
  }

  void Presolve::updateStateRowSingletonInequality(
    const int row_index, const int col_index
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
  }

  void Presolve::applyRowSingletonPostsolve(const int row_index){
    // Since row singleton equality leads to a fixed column 
    // in postsolve the feasible value will already have been
    // found by applyFixedColPostsolve, so we just have to turn 
    // the row on on postsolve so that it can be included in the 
    // constraints checks.
    if (isRowActivePostsolve(row_index)) {
      if (checkConstraint(row_index, static_cast<int>(core::PresolveRulesIds::rowSingletonId))) {
        postsolve_active_rows_.at(row_index) = true;
      }
    }
  }

  bool Presolve::checkAreRowsParallel(
    const int row1_index, const int row2_index
  ) {
    if (rows_non_zero_variables_.at(row1_index).size() != rows_non_zero_variables_.at(row2_index).size()) {
      return false;
    }
    int row1_first_non_zero_col = rows_non_zero_variables_.at(row1_index).at(0);
    int row2_first_non_zero_col = rows_non_zero_variables_.at(row2_index).at(0);

    if (row1_first_non_zero_col != row2_first_non_zero_col) {
      return false;
    }
    double ratio = static_cast<double>(problem_matrix_.at(row1_index).at(row1_first_non_zero_col))/problem_matrix_.at(row2_index).at(row2_first_non_zero_col);

    for (std::size_t j = 1; j < rows_non_zero_variables_.at(row1_index).size(); ++j) {
      int row1_non_zero_col = rows_non_zero_variables_.at(row1_index).at(j);
      int row2_non_zero_col = rows_non_zero_variables_.at(row2_index).at(j);

      if (row1_non_zero_col != row2_non_zero_col) {
        return false;
      }

      double ratio_new = static_cast<double>(problem_matrix_.at(row1_index).at(row1_non_zero_col))/problem_matrix_.at(row2_index).at(row2_non_zero_col);
      if (ratio != ratio_new) {return false;}
      ratio = ratio_new;
    }

    return true;
  }

  int Presolve::getParallelRow(const int row_index, const int start) {
    if (row_index == start) {return -1;}
    
    for (std::size_t k = start; k < row_index; ++k) {
      if (checkAreRowsParallel(row_index, k)) {return k;}
    }

    return -1;
  }

  std::vector<int> Presolve::sortParallelRowsBySize(
    const int row, const int parallel_row
  ) {
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
  }

  bool Presolve::checkAreParallelRowsFeasible(
    const int small_row_index,
    const int large_to_small_ratio,
    const double large_lower_bound_by_ratio
  ){
    // If we are dealing with an equality, check if the bounds
    // are also multiples of eachother and if they aren't
    // then the system is not feasible.
    if (small_row_index >= inequalities_count_) {
      if (large_lower_bound_by_ratio != lower_bounds_.at(small_row_index)) {
        return false;
      }
    }
    // If we have one row of each sign, we now have to check feasibility
    // for inequalities.
    if (large_to_small_ratio < 0) {
      // large bound by ratio (rounded down to ensure feasibility for
      // integers if solving ip) now becomes an upper bound of the 
      // small inequality. Thus, if it is smaller than the small row's 
      // bound, the problem is infeasible.
      if (solve_ip_) {
        if (std::floor(large_lower_bound_by_ratio) < lower_bounds_.at(small_row_index)) {
          return false;
        }
      } else {
        if (large_lower_bound_by_ratio < lower_bounds_.at(small_row_index)) {
          return false;
        }
      }
    }
    return true;

  }

  void Presolve::updateStateParallelRow(
    const int small_row_index, 
    const int large_row_index,
    const double large_to_small_ratio,
    const double large_lower_bound_by_ratio
  ) {
    // turn off large row and log into stack
    presolve_active_rows_.at(large_row_index) = false;
    presolve_active_rows_count_ -= 1;
    struct presolve_log log = {
      large_row_index, -1, 
      static_cast<int>(core::PresolveRulesIds::parallelRowId)
    };
    presolve_stack_.push(log);

    // if we have an inequality, update the bound on the small row 
    // to ensure that both are satisfied.
    if (small_row_index < inequalities_count_) {
      // If we are solving the ip then we have that we 
      // need to round up the large row's lower bound divided by 
      // the ratio to ensure that constraints are satisfied when 
      // restricted to integers.
      double potential_lower_bound;
      if (solve_ip_) {potential_lower_bound = std::ceil(large_lower_bound_by_ratio);}
      else {potential_lower_bound = large_lower_bound_by_ratio;}

      int small_row_lower_bound = lower_bounds_.at(small_row_index);

      if (large_to_small_ratio > 0) {
        // If the inequalities both have the same sign then we use 
        // the bound that takes up all the slack.
        if (small_row_lower_bound > potential_lower_bound) {
          lower_bounds_.at(small_row_index) = potential_lower_bound;
        }
      } else {
        // If they do not have the same sign, then that means that the 
        // lower bound of the large one divided by the large 
        // to small ratio (rounded down if we are solving the ip), 
        // will have become an upper bound of the small one - if 
        // this upper bound is smaller than the small one's
        // lower bound, we will have already deemed the system unfeasible in 
        // checkAreParallelRowsFeasible.
        if (solve_ip_) {
          upper_bounds_.at(small_row_index) = std::floor(large_lower_bound_by_ratio);
        } else {
          upper_bounds_.at(small_row_index) = large_lower_bound_by_ratio;
        }
      }
    }
  }

  void Presolve::applyParallelRowPostsolve(const int row_index) {
    if (isRowActivePostsolve(row_index)) {
      if (checkConstraint(row_index, static_cast<int>(core::PresolveRulesIds::parallelRowId))) {
        postsolve_active_rows_.at(row_index) = true;
      }
    }
  }

  void Presolve::updateStateEmptyCol(const int col_index) {
    presolve_active_columns_.at(col_index) = false;
    presolve_active_cols_count_ -= 1;

    struct presolve_log log = {
      -1, col_index, 
      static_cast<int>(core::PresolveRulesIds::emptyColId)
    };
    presolve_stack_.push(log);
  }

  void Presolve::applyEmptyColPostsolve(const int col_index) {
    feasible_solution_.at(col_index) = 0;
    postsolve_active_cols_.at(col_index) = true;
  }

  bool Presolve::isFixedCol(const int col_index) {
    if (implied_lower_bounds_.at(col_index) == implied_upper_bounds_.at(col_index)) {
      return true;
    }
    return false;
  }

  void Presolve::updateStateFixedCol(const int col_index) {
    int variable_value = implied_lower_bounds_.at(col_index);
    // Update the lower bound of each constraint that 
    // contains the variable using the variable value.
    for (auto&i : cols_non_zeros_indices_.at(col_index)) {
      int coefficient = problem_matrix_.at(i).at(col_index);
      int lower_bound = lower_bounds_.at(i);
      lower_bounds_.at(i) = lower_bound - coefficient * variable_value;
    }

    presolve_active_columns_.at(col_index) = false;
    presolve_active_cols_count_ -= 1;
    // Log -1 in row index as not applicable in this 
    // rule.
    struct presolve_log log = {
      -1, col_index, 
      static_cast<int>(core::PresolveRulesIds::fixedColId), 
      cols_non_zeros_indices_.at(col_index)
    };
    presolve_stack_.push(log);
  }

  void Presolve::applyFixedColPostsolve(
    const int col_index, const std::vector<int> col_non_zeros
  ) {
    // In postsolve we know that the feasible value 
    // is feasible because when updating the implied 
    // bounds in presolve we check that the implied 
    // bounds we get are integers.
    int feasible_value = implied_lower_bounds_.at(col_index);

    // Re-update the lower bound of each constraint that 
    // contained the variable at the time it was identified 
    // as fixed using the variable value.
    for (auto&i : col_non_zeros) {
      int coefficient = problem_matrix_.at(i).at(col_index);
      int lower_bound = lower_bounds_.at(i);
      lower_bounds_.at(i) = lower_bound + coefficient * feasible_value;
    }

    postsolve_active_cols_.at(col_index) = true;
    feasible_solution_.at(col_index) = feasible_value;
  }

  bool Presolve::isFreeColSubstitution(
    const int row_index, const int col_index
  ) {
    if (implied_lower_bounds_.at(col_index) == -core::kIntInfinity && implied_upper_bounds_.at(col_index) == core::kIntInfinity) {
      return true;
    }
    return false;
  }

  void Presolve::updateStateFreeColSubstitution(
    int row_index, int col_index
  ) {
    // Turn off row and col
    presolve_active_rows_.at(row_index) = false;
    presolve_active_columns_.at(col_index) = false;
    presolve_active_rows_count_ -= 1;
    presolve_active_cols_count_ -= 1;

    // Update presolve stack.
    struct presolve_log log = {
      row_index, col_index, 
      static_cast<int>(core::PresolveRulesIds::freeColSubsId)
    };
    presolve_stack_.push(log);
  }

  double Presolve::getFreeColSubstitutionSumOfDependancies(
    const int row_index, const int col_index
  ) {
    double sum_of_dependancies = 0;

    for (std::size_t j = 0; j < variables_count_; ++j) {
      // If it is not the singleton column, check if the coefficient
      // in the problem is non-zero, and if it isn't if a feasible value
      // has been found update sum of dependancies, and it not return 
      // core::kIntInfinity.
      if (j != col_index) {
        int col_coefficient = problem_matrix_.at(row_index).at(j);

        if (col_coefficient != 0) {
          if (postsolve_active_cols_.at(j))  {
            sum_of_dependancies += col_coefficient*feasible_solution_.at(j);
          } else {
            return core::kIntInfinity;
          }
        }
      }
    }
    return sum_of_dependancies;
  }

  void Presolve::applyFreeColSubstitutionPostsolve(
    const int row_index, const int col_index
  ) {
    // Check if a feasible value has been found for a 
    // variable, and if so apply postsolve.
    double sum_of_dependancies = getFreeColSubstitutionSumOfDependancies(
      row_index, col_index
    );
    double feasibleValueCalculationBound = getFeasibleValueCalculationBound(
      row_index
    );

    if (sum_of_dependancies != core::kIntInfinity) {
      double RHS = feasibleValueCalculationBound - sum_of_dependancies;
      int variable_coefficient = problem_matrix_.at(row_index).at(col_index);
      double feasible_value;

      if (solve_ip_) {
        feasible_value = getVariableFeasibleValueIP(
          row_index, col_index, variable_coefficient, RHS
        );
      } else {
        feasible_value = RHS/variable_coefficient;
      }

      if (feasible_value == core::kIntInfinity) {infeasible_ = true;}
      else {
        feasible_solution_.at(col_index) = feasible_value;
        postsolve_active_cols_.at(col_index) = true;
        // Check if we have found feasible values for all variables in the 
        // row. If so, check if the row (constraint) is satisfied, and if 
        // so turn on row in postsolve.
        if (isRowActivePostsolve(row_index)) {
          if (checkConstraint(row_index, 6)) {
            postsolve_active_rows_.at(row_index) = true;
          }
        }
      }
    }
  }

  void Presolve::applyPresolveRowRules() {
    for (std::size_t i = 0; i < constraints_count_; ++i) {
      // If row is active, apply row rules.
      if (presolve_active_rows_.at(i)) {
        // Check if it is a free row, and if so update state
        // accordingly and continue to the next iteration.
        if (checkIsRowFree(i)) {
          updateStateFreeRow(i);
          continue;
        }
        // Check if row is parallel to another row from row 0
        // to row i-1, or from the start of the equalities to i-1.
        int parallel_row_search_start = 0;
        if (i >= inequalities_count_) {
          parallel_row_search_start = inequalities_count_;
        }
        int parallel_row = getParallelRow(i, parallel_row_search_start);

        // If parallel row is found, check feasibility and 
        // if feasible call updateSateParallelRows.
        if (parallel_row != -1) {
          std::vector<int> sorted_rows = sortParallelRowsBySize(i, parallel_row);
          int small_row_index = sorted_rows.at(0);
          int large_row_index = sorted_rows.at(1);
          double large_to_small_ratio = static_cast<double>(problem_matrix_.at(large_row_index).at(rows_non_zero_variables_.at(large_row_index).at(0)))/problem_matrix_.at(small_row_index).at(rows_non_zero_variables_.at(small_row_index).at(0));
          double large_lower_bound_by_ratio = lower_bounds_.at(large_row_index)/large_to_small_ratio;

          // If parallel row not feasible, set problem to infeasible
          // and break, else update state.
          if (!checkAreParallelRowsFeasible(
            small_row_index, large_to_small_ratio, 
            large_lower_bound_by_ratio)
          ) {
            infeasible_ = true;
            infeasible_by_PR_ = true;
            break;
          } else {
            updateStateParallelRow(
              small_row_index, large_row_index, 
              large_to_small_ratio, large_lower_bound_by_ratio
            );
          }
          // if row i was the large row in parallel rows it will have 
          // been turned off so we don't check the rest of the rules.
          if (large_row_index == i) {continue;}
        }
        int row_non_zeros_count = rows_non_zero_variables_.at(i).size();

        // If row is a row singleton, check if it is a singleton 
        // variable, row singleton equality or row singleton inequality,
        // and update state accordingly.
        if (row_non_zeros_count == 1) {
          int non_zero_variable = rows_non_zero_variables_.at(i).at(0);
          int corresponding_col_non_zeros_count = cols_non_zeros_indices_.at(
            non_zero_variable
          ).size();

          // If it is a singleton variable, update state accordingly.
          if (corresponding_col_non_zeros_count == 1) {
            updateStateRowAndColSingleton(i, non_zero_variable);
          }
          
          // If it is not a singleton variable, check if is an equality
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
  }

  void Presolve::applyPresolveColRules() {
    for (std::size_t j = 0; j < variables_count_; ++j) {
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
  }

  bool Presolve::checkVariableImpliedBounds(
    const int col_index, const int feasible_value
  ) {
    if (feasible_value < implied_lower_bounds_.at(col_index) || feasible_value > implied_upper_bounds_.at(col_index)) {
      // printf("Variable %d does not meet the implied bounds.\n", col_index);
      // printf("Implied bounds: Lower = %d, Upper = %d.\n", implied_lower_bounds_.at(col_index), implied_upper_bounds_.at(col_index));
      return false;
    }
    return true;
  }

  bool Presolve::isRowActivePostsolve(const int row_index) {
    for (std::size_t j = 0; j < variables_count_; ++j) {
      if (problem_matrix_.at(row_index).at(j) != 0) {
        if (!postsolve_active_cols_.at(j)) {
          return false;
        }
      }
    }
    return true;
  }

  void Presolve::setPrintUnsatisfiedConstraints() {
    print_unsatisfied_constraints_ = true;
  }

  bool Presolve::checkConstraint(
    const int row_index, const int rule_id
  ) {
    double constraint_value = 0;

    // Loop through row active columns working out the 
    // constraint value.
    for (std::size_t j = 0; j < variables_count_; ++j) {
      if (postsolve_active_cols_.at(j)) {
        constraint_value += problem_matrix_.at(row_index).at(j)*feasible_solution_.at(j);
      }
    }

    // Check if constraint is satisfied, and if not 
    // add index to unsatisfied constraints vector.
    if (constraint_value < lower_bounds_.at(row_index) || constraint_value > upper_bounds_.at(row_index)) {
      if (print_unsatisfied_constraints_) {
        std::cout<<""<<std::endl;
        printf(
          "Constraint %d was unsatisfied after applying rule %d.\n",
          row_index, rule_id
        );
        printf("Constraint %d value: %f\n", row_index, constraint_value);
        printf(
          "Lower bound: %f; upper bound: %f\n", 
          lower_bounds_.at(row_index),
          upper_bounds_.at(row_index)
        );
        std::cout<<""<<std::endl;
      }
      unsatisfied_constraints_ = true;
      return false;
    }
    return true;
  }

  void Presolve::applyPresolve() {
    int iteration_active_rows = presolve_active_rows_count_;
    int iteration_active_cols = presolve_active_cols_count_;
    while (presolve_active_rows_count_ > 0) {
      getRowsAndColsNonZeros();
      applyPresolveRowRules();
      applyPresolveColRules();
      if (infeasible_) {break;}
      
      if (presolve_active_rows_count_ == iteration_active_rows && presolve_active_cols_count_ == iteration_active_cols) {
        break;
      } else {
        iteration_active_rows = presolve_active_rows_count_;
        iteration_active_cols = presolve_active_cols_count_;
      }
    }

    if (presolve_active_rows_count_ == 0 && presolve_active_cols_count_ == 0) {
      reduced_to_empty_ = true;
    }
  }

  void Presolve::applyPostsolve() {
    postsolve_active_rows_.resize(constraints_count_, false);
    postsolve_active_cols_.resize(variables_count_, false);
    if (!infeasible_) {
      while (!presolve_stack_.empty()) {
        presolve_log rule_log = presolve_stack_.top();
        int rule_id = rule_log.rule_id;
        int row_index = rule_log.constraint_index;
        int col_index = rule_log.variable_index;
        
        if (rule_id == static_cast<int>(core::PresolveRulesIds::freeRowId)) {
          applyFreeRowPostsolve(row_index);
        }
        else if (rule_id == static_cast<int>(core::PresolveRulesIds::rowAndColSingletonId)) {
          applyRowAndColSingletonPostsolve(row_index, col_index);
        }
        else if (rule_id == static_cast<int>(core::PresolveRulesIds::rowSingletonId)) {
          applyRowSingletonPostsolve(row_index);
        }
        else if (rule_id == static_cast<int>(core::PresolveRulesIds::parallelRowId)) {
          applyParallelRowPostsolve(row_index);
        }
        else if (rule_id == static_cast<int>(core::PresolveRulesIds::emptyColId)) {
          applyEmptyColPostsolve(col_index);
        }
        else if (rule_id == static_cast<int>(core::PresolveRulesIds::fixedColId)) {
          applyFixedColPostsolve(col_index, rule_log.dependancies);
        } 
        else if (rule_id == static_cast<int>(core::PresolveRulesIds::freeColSubsId)) {
          applyFreeColSubstitutionPostsolve(row_index, col_index);
        }

        if (infeasible_) {
          break;
        }
        // Remove rule from stack.
        presolve_stack_.pop();
      }
    }
  }

  void Presolve::printFeasibleSolution() {
    for (int i = 0; i < variables_count_; ++i) {
      printf("Variable %d = %f\n", i, feasible_solution_.at(i));
    }
    std::cout<<" "<<std::endl;
  }

  void Presolve::printBounds() {
    std::cout<<"Constraints Bounds"<<std::endl;;

    for (std::size_t i = 0; i < constraints_count_; ++i) {
      int lower_bound = lower_bounds_.at(i);
      int upper_bound = upper_bounds_.at(i);

      std::string lower_str;
      std::string upper_str;

      if (lower_bound == -core::kIntInfinity) {
        lower_str = "-Inf";
      } else {
        lower_str = std::to_string(lower_bound);
      }

      if (upper_bound == core::kIntInfinity) {
        upper_str = "Inf";
      } else {
        upper_str = std::to_string(upper_bound);
      }
      
      std::cout << lower_str << ", " << upper_str << std::endl;
    }

    std::cout<<" "<<std::endl;
  }

  void Presolve::printImpliedBounds() {
    std::cout<<"Implied Bounds"<<std::endl;;

    for (std::size_t i = 0; i < variables_count_; ++i) {
      int lower_bound = implied_lower_bounds_.at(i);
      int upper_bound = implied_upper_bounds_.at(i);

      std::string lower_str;
      std::string upper_str;

      if (lower_bound == -core::kIntInfinity) {
        lower_str = "-Inf";
      } else {
        lower_str = std::to_string(lower_bound);
      }

      if (upper_bound == core::kIntInfinity) {
        upper_str = "Inf";
      } else {
        upper_str = std::to_string(upper_bound);
      }
      
      std::cout << lower_str << ", " << upper_str << std::endl;
    }

    std::cout<<" "<<std::endl;
  }

  void Presolve::printRow(const int row_index) {
    std::cout<<""<<std::endl;
    printf("Row %d:", row_index);
    for (const int& e : problem_matrix_.at(row_index)) {
        std::cout << e << " ";
    }
    std::cout << std::endl;
  }

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
  }

  void Presolve::printPresolveCurrentState() {
    for (int i = 0; i < constraints_count_; ++i) {
      if (rows_non_zero_variables_.at(i).size() > 0) {
        printf("ROW %d\n", i);

        for (int j = 0; j < rows_non_zero_variables_.at(i).size(); ++j) {
          int col_index = rows_non_zero_variables_.at(i).at(j);
          printf("Col %d: %d\n", col_index, problem_matrix_.at(i).at(col_index));
        }
        std::cout<<""<<std::endl;
      }
    }
  }
}
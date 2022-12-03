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

    std::fill(active_rows_.begin(), active_rows_.end(), true);
    std::fill(active_columns_.begin(), active_columns_.end(), true);
  };

  int Presolve::search_row_singleton_equality(std::vector<int> row) {
    int row_singleton = -1;

    for (int i = 0; i < variables_count_; ++i) {
      int row_element = row.at(i);

      if (row_element != 0) {
        // if no non-0 had been found set row singleton
        // to variable found.
        if (row_singleton == -1) {
          row_singleton = i;
        } 
        // Else, more than one variable is non-zero 
        // so row is not a row singleton.
        else {
          return -1;
        }
      }
    }
    return row_singleton;
  };

  void Presolve::update_state_row_singleton_equality(
    int row_index, int variable_index
  ) {
    int variable_value = lower_bounds_.at(row_index);

    // Set variable equal to its value in the rest of the 
    // rows.
    for (int i = 0; i < constraints_count_; i++) {
      problem_matrix_.at(i).at(variable_index) = variable_value;
    }

    // turn off corresponding problem row.
    active_rows_.at(row_index) = false;

    // Get pointer to corresponding postsolve function.
    void (Presolve::*postsolve_function_pointer) (int, int);
    postsolve_function_pointer = &Presolve::apply_row_singleton_equality_postsolve;

    // Update presolve stack.
    struct presolve_log log = {
      row_index, variable_index, 
      postsolve_function_pointer
    };
    
  };

  void Presolve::apply_row_singleton_equality_postsolve(
    int row_index, int variable_index
  ){
    // This was already found in the update state function for this
    // particular rule, but doing it again to maintain consistency
    // of postsolve functions' functionality.
    int variable_feasible_solution = lower_bounds_.at(row_index);

    feasible_solution.at(variable_index) = variable_feasible_solution;
    active_rows_.at(row_index) = true;
  };

  void Presolve::apply_presolve() {
    // Apply row equality rules.
    int equalities_start = constraints_count_ - equalities_count_; 

    for (int i = equalities_start; i < constraints_count_; ++i) {
      std::vector<int> row = problem_matrix_.at(i);
      int row_singleton = search_row_singleton_equality(row);

      // If row is a row singleton, update state accordingly.
      if (row_singleton != -1) {
        update_state_row_singleton_equality(i, row_singleton);
      }
    }
  };

  void Presolve::apply_postsolve() {
    while (!presolve_stack_.empty()) {
        presolve_log rule_log = presolve_stack_.top();
        Presolve::rule_log.postsolve_function(rule_log.constraint_index, rule_log.variable_index);
    }
  }

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
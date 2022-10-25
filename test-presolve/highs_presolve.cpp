#include "Highs.h"
#include "../lib/utils/reader.hpp"
#include <vector>
#include <iostream>
// #include <stdint.h>
#include <string>
#include "../lib/core/types.hpp"

const int all_test_cases_count = 150218;
const bool test_all = false;
const int to_test_count = 1;
int reduced_to_empty_count = 0;
std::vector<string> used_presolve_rules;


// HiGHS variables:
const double inf = kHighsInf;


/// @brief Reads and presolves one problem from the test cases
/// using reader.cpp and HiGHS. 
/// @param highs - HiGHS object.
/// @param reader_ - Reader object.
/// @param problem_number is the problem to presolve.
void presolveProblem(
  Highs& highs,
  const std::vector<std::vector<int>> problem_matrix,
  const std::vector<int> lower_bounds,
  const std::vector<int> upper_bounds
);


/// @brief For test problems 1 to N reads the problems using reader.cpp
/// and applies and reports presolve using HiGHS.
/// @param n the number of problems to read and presolve.
void presolveProblemsN(const int n);


/// @brief Defines a problem in a HiGHS instance.
/// @param highs - HiGHS object
/// @param problem_matrix - Matrix of problem constraints.
/// @param upprer_bounds - int vector of upper bounds for the constraints.
/// @param lower_bounds - int vector of lower bounds for the constraints.
/// @return HighsStatus to indicate whether the definition was successful.
HighsStatus defineLp(
  Highs& highs, 
  const std::vector<std::vector<int>> problem_matrix,
  const std::vector<int> lower_bounds,
  std::vector<int> upper_bounds
);


void reportAndLogPresolveLog(Highs& highs);


/// @brief Returns the bounds to be entered to HiGHS.
/// @param constant_term is the constant term of the constraint.
/// @param upper_bound is the upper bound of the constraint.
/// @param lower_bound is the lower bound of the constraint.
/// @return double vector with lowe and upper bound. Both 0 if
/// <upper_bound> = <lower_bound>, [-<constant_term>, inf] otherwise.
std::vector<double> getHighsRowBounds(
  const int constant_term,
  const int lower_bound,
  const int upper_bound
);


void reportAndLogPresolveLog(Highs& highs) {
  const HighsPresolveLog& presolve_log = highs.getPresolveLog();
  // The presolve_rule_off option will alow certain presolve rules to
  // be switched off
  int presolve_rule_off = highs.getOptions().presolve_rule_off;
  int bit = 1;
  printf("\nRule  Bit| Call Row Col| Name\n");

  for (int rule_ix = 0; rule_ix < kPresolveRuleCount; rule_ix++) {
    bool allow = !(presolve_rule_off & bit);
    const HighsPresolveRuleLog& log = presolve_log.rule[rule_ix];
    const std::string presolve_rule = highs.presolveRuleTypeToString(rule_ix);

    if (log.call) printf("  %2d %4d|  %3d %3d %3d| %s\n", rule_ix, bit,
			 log.call, 
			 log.row_removed,
			 log.col_removed,
			 presolve_rule.c_str());
    bit *= 2;

    if (std::find(used_presolve_rules.begin(), used_presolve_rules.end(), presolve_rule) != used_presolve_rules.end()) {
      used_presolve_rules.push_back(presolve_rule);
    }
  }
};


std::vector<double> getHighsRowBounds(
  const int constant_term,
  const int lower_bound,
  const int upper_bound
) {
  std::vector<double> highs_bounds; 

  if ((upper_bound == 0) && lower_bound == 0) {
     highs_bounds.push_back((double)(-constant_term));
     highs_bounds.push_back((double)(-constant_term));

     return highs_bounds;
  }

  const double highs_lower_bound = (double)(-constant_term);
  highs_bounds.push_back(highs_lower_bound);
  highs_bounds.push_back(inf);

  return highs_bounds;
};


HighsStatus defineLp(
  Highs& highs, 
  const std::vector<std::vector<int>> problem_matrix,
  const std::vector<int> lower_bounds,
  const std::vector<int> upper_bounds
) {
  HighsStatus return_status = HighsStatus::kOk;

  // Problem dimensions.
  const int num_var = problem_matrix[0].size();
  const int num_rows = problem_matrix.size();

  // Setting up problem bounds
  std::vector<double> lower;
  std::vector<double> upper;
  lower.assign(num_var, -inf);
  upper.assign(num_var, inf);
  return_status = highs.addVars(num_var, &lower[0], &upper[0]);

  for (int row_num = 0;  row_num < num_rows; ++row_num) {
    // Getting row, constant term and bounds for the row.
    std::vector<int> row = problem_matrix[row_num];
    int constant_term = row[0];
    int lower_bound = lower_bounds[row_num];
    int upper_bound = upper_bounds[row_num];
    std::vector<double> highs_bounds = getHighsRowBounds(constant_term, lower_bound, upper_bound);

    //non-zero indices and values.
    std::vector<int> indices;
    std::vector<double> values;

    for (int i = 0; i < num_var; ++i) {
      int coeff = row[i];
      
      // If coefficient is non-zero, update indices and values.
      if (coeff != 0) {
        indices.push_back(i);
        values.push_back(coeff);
      }
    }
    
    // Add row to HiGHS.
    int num_nz = indices.size();
    return_status = highs.addRow(highs_bounds[0], highs_bounds[1], num_nz, &indices[0], &values[0]);
  }

  return return_status;
};


void presolveProblem(
  Highs& highs,
  const std::vector<std::vector<int>> problem_matrix,
  const std::vector<int> lower_bounds,
  const std::vector<int> upper_bounds
  ) {

  // Useful references to HiGHS variables.
  const HighsInfo& info = highs.getInfo();
  const HighsLp& lp = highs.getLp();
  const HighsLp& presolved_lp = highs.getPresolvedLp();
  const HighsSolution& solution = highs.getSolution();
  // Convenient short-hand for the number of rows
  const int num_row = lp.num_row_;

  // Define problem into HiGHS.
  HighsStatus return_status = defineLp(highs, problem_matrix, lower_bounds, upper_bounds);
  assert(return_status == HighsStatus::kOk);

  return_status = highs.presolve();
  assert(return_status == HighsStatus::kOk);
  // Flag up case where presolve does not reduce to empty
  if (!(presolved_lp.num_row_+presolved_lp.num_col_)) {
    reduced_to_empty_count += 1;
    reportAndLogPresolveLog(highs);
  }
};


void presolveProblemsN(int n) {
  // Instantiate HiGHS and define variable for
  // HiGHS status.
  Highs highs;
  HighsStatus return_status;

  // Set HiGHS running options.
  return_status = highs.setOptionValue("output_flag", false);
  assert(return_status == HighsStatus::kOk);
  highs.setOptionValue("presolve_log_report", true);
  highs.setOptionValue("doubleton_equation_sudoku_report", true);

  //Instanciate reader.
  utils::Reader reader;
  std::string test_problems = "/Users/pepe/hons-project/problems/feasibility_testcases.txt";

  for (int i = 0; i < n; ++i) {
    // Reading problem.
    reader.readProblem(test_problems, i);
    std::vector<std::vector<int>> mat = reader.problem_matrix_;
    std::vector<int> max = reader.upper_bounds_;
    std::vector<int> min = reader.lower_bounds_;

    presolveProblem(highs, mat, min, max);
    highs.clearModel();
  }
};


int main() {
  if (test_all) {
    presolveProblemsN(all_test_cases_count);
  } else {
    presolveProblemsN(to_test_count);
  }

  for(int i = 0; i < used_presolve_rules.size(); ++i){
      std::cout << used_presolve_rules[i] << std::endl;
  }

  return 0;
};
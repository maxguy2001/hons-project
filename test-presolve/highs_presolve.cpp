#include "../lib/utils/reader.hpp"
#include "Highs.h"
#include <vector>
#include <iostream>
#include <string>
#include <fstream>


// Constants:

std::string test_problems = "/Users/pepe/hons-project/problems/feasibility_testcases.txt";
const int all_test_cases_count = 150218;
const double inf = kHighsInf;

// Parameters:

// Set test_all to true to test all problems.
const bool test_all = true; 
const int to_test_count = 3000;
bool print_problems = false;//true;

// Report variables:
int reduced_to_empty_count = 0;
std::vector<string> used_presolve_rules;


/// @brief Prints a test case problem. 
/// @param problem_matrix - matrix for the problem constraints.
/// @param lower_bounds - lower bounds for problem constraints.
/// @param upper_bounds - upper bounds for problem constraints.
void printLP(
  const std::vector<std::vector<int>> problem_matrix,
  const std::vector<int> lower_bounds,
  const std::vector<int> upper_bounds
);


/// @brief Returns whether a presolve rule has been used.
/// @param presolve_rule 
/// @return Boolean indicating whether presolve_rule has been
/// found in used rules.
bool findPresolveRuleInUsed(const std::string presolve_rule);


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


/// @brief Reads and presolves one problem from the test cases
/// using reader.cpp and HiGHS. 
/// @param highs - HiGHS object.
/// @param problem_matrix - matrix for the problem constraints.
/// @param lower_bounds - lower bounds for problem constraints.
/// @param upper_bounds - upper bounds for problem constraints.
void presolveSingleProblem(
  Highs& highs,
  const std::vector<std::vector<int>> problem_matrix,
  const std::vector<int> lower_bounds,
  const std::vector<int> upper_bounds
);


/// @brief For test problems 1 to to problems_count reads the problems
/// using reader.cpp and applies and reports presolve using HiGHS.
/// @param n the number of problems to read and presolve.
void presolveProblems(const int problems_count);


void printLP(
  const std::vector<std::vector<int>> problem_matrix,
  const std::vector<int> lower_bounds,
  const std::vector<int> upper_bounds
) {
    for (int i = 0; i < problem_matrix.size(); ++i) {
      printf("Row %d \n", i+1);

      for (int j = 0; j < problem_matrix[0].size(); ++j) {
        std::cout << problem_matrix[i][j] << std::endl;
      }
      std::cout<<" "<<std::endl;
    }

    std::cout<<" "<<std::endl;
    std::cout<<"Bounds"<<std::endl;;

    for (int i = 0; i < problem_matrix.size(); ++i) {
      std::string upper_bound = std::to_string(upper_bounds[i]);
      if (upper_bound == "32765") {
        upper_bound = "Inf";
      }
      std::cout << lower_bounds.at(i) << ", " << upper_bound << std::endl;
    }
}


bool findPresolveRuleInUsed(const std::string presolve_rule) {
  for (auto&used_rule : used_presolve_rules) {
    if (used_rule == presolve_rule) {
      return true;
    }
  }
  return false;
}


void reportAndLogPresolveLog(Highs& highs) {
  const HighsPresolveLog& presolve_log = highs.getPresolveLog();
  // The presolve_rule_off option will alow certain presolve rules to
  // be switched off

  if (print_problems) {
    printf("\nRule  Bit| Call Row Col| Name\n");
  }
  int bit = 1;
  for (int rule_ix = 0; rule_ix < kPresolveRuleCount; rule_ix++) {
    const HighsPresolveRuleLog& log = presolve_log.rule[rule_ix];
    const std::string presolve_rule = highs.presolveRuleTypeToString(rule_ix);

    if (log.call) {
      if (print_problems) {
        printf("  %2d %4d|  %3d %3d %3d| %s\n", rule_ix, bit,
        log.call, 
        log.row_removed,
        log.col_removed,
        presolve_rule.c_str());
      }

      if (!findPresolveRuleInUsed(presolve_rule)) {
        used_presolve_rules.push_back(presolve_rule);
      }
    }
    bit *= 2; 
  }
};


HighsStatus defineLp(
  Highs& highs, 
  const std::vector<std::vector<int>> problem_matrix,
  const std::vector<int> lower_bounds,
  const std::vector<int> upper_bounds
) {
  HighsStatus return_status = HighsStatus::kOk;

  // Problem dimensions.
  // Since problems with no rows shouldn't reach here, add an assert
  const int num_rows = problem_matrix.size();
  assert(num_rows > 0);
  const int num_var = problem_matrix[0].size();

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
    int upper_bound;

    if (upper_bounds[row_num] == 32765) {
      upper_bound = kHighsIInf;// JAJH This was inf, but that's a double
    }

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
    return_status = highs.addRow(lower_bound, upper_bound, num_nz, &indices[0], &values[0]);
  }

  return return_status;
};


void presolveSingleProblem(
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


void presolveProblems(int problems_count) {
  // Instantiate HiGHS and define variable for
  // HiGHS status.
  Highs highs;
  HighsStatus return_status;

  // Set HiGHS running options.
  return_status = highs.setOptionValue("output_flag", false);
  assert(return_status == HighsStatus::kOk);
  highs.setOptionValue("presolve_log_report", true);

  // Instantiate reader.
  utils::Reader reader;

  // Start filestream to pass to reader.
  std::fstream problems_filestream;
  problems_filestream.open(test_problems, std::ios::in);

  // JAJH: Inserting "&" makes problem_matrix, upper_bounds and
  // lower_bounds references to the corresponding vector in reader,
  // otherwise the data are copied every time
  std::vector<std::vector<int>>& problem_matrix = reader.problem_matrix_;
  std::vector<int>& upper_bounds = reader.upper_bounds_;
  std::vector<int>& lower_bounds = reader.lower_bounds_;

  int num_empty_problems = 0;
  for (int n = 0; n < problems_count; n++) {
    if (n % 1000 == 0) printf("Reading problem %6d\n", n);

    // Reading problem.
    reader.readNextProblem(problems_filestream);

    // Ignore test problems with no rows
    int num_row = problem_matrix.size();
    if (num_row <= 0) {
      assert(num_row == 0);
      num_empty_problems++;
      continue;
    }

    // If any problems have no variables,
    // flag them up with assert and ignore them
    int num_var = problem_matrix[0].size();
    assert(num_var > 0);
    if (num_var <= 0) {
      assert(num_var == 0);
      printf("Ignore test problems with %d variables\n", num_var);
      continue;
    }

    if (print_problems) {
      std::cout<<" "<<std::endl;
      std::cout<<" "<<std::endl;
      printf("Problem %d \n", n+1);
      std::cout<<" "<<std::endl;
      printLP(problem_matrix, lower_bounds, upper_bounds);
      std::cout<<" "<<std::endl;
    }

    presolveSingleProblem(highs, problem_matrix, lower_bounds, upper_bounds);
    highs.clearModel();
  }

  problems_filestream.close();
  printf("Ignored %d empty test problems\n", num_empty_problems);
};


int main() {
  int problems_to_test;

  if (test_all) {
    problems_to_test = all_test_cases_count;
    print_problems = false;
  } else {
    problems_to_test = to_test_count;
  }

  presolveProblems(problems_to_test);

  std::cout<<" "<<std::endl;
  std::cout<<" "<<std::endl;
  std::cout<<"Presolve rules used:"<<std::endl;
  std::cout<<""<<std::endl;
  for(int i = 0; i < used_presolve_rules.size(); ++i){
      std::cout << used_presolve_rules[i] << std::endl;
  }

  std::cout<<" "<<std::endl;  
  printf("Problems tested: %d \n", problems_to_test);
  printf("Problems reduced to empty: %d \n", reduced_to_empty_count);

  return 0;
}
#include "../../core/consts.hpp"
#include "../../core/types.hpp"
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <limits>
#include <stack>
#include <string>
#include <vector>

namespace logical_solver {

class Presolve {
public:
  // PUBLIC CLASS MEMBERS:
  // Boolean to check if problem has been reduced to empty.
  bool reduced_to_empty_;
  bool infeasible_;
  bool infeasible_by_PR_;
  bool unsatisfied_constraints_;
  bool print_unsatisfied_constraints_;

  // CONSTRUCTOR
  Presolve(
    const std::vector<std::vector<int>> problem_matrix, 
    const std::vector<int> lower_bounds, 
    const std::vector<int> upper_bounds,
    const int inequalities_count,
    const int equalities_count,
    const bool solve_ip
  );

  // PUBLIC METHODS
  /**
   * @brief Applies presolve to the problem.
   *
   * @return void.
   */
  void applyPresolve();

  /**
   * @brief Applies postsolve to the problem.
   *
   * @return void.
   */
  void applyPostsolve();

  /**
   * @brief Prints the feasible solutions vector.
   */
  void printFeasibleSolution();

  /**
   * @brief Prints the instance LP.
   */
  void printLP();

  /**
   * @brief Prints the constraints bounds.
   */
  void printBounds();

  /**
   * @brief Prints the implied bounds.
   */
  void printImpliedBounds();

  void printRow(const int row_index);

  /**
   * @brief Public method to check if the feasible solution
   * of a problem is integer valued.
   *
   * @return Bool: whether or not all variables in the feasible
   * solution are integers.
   */
  bool isFeasibleSolutionInteger();

  /**
   * @brief Called if user wished presolve to print
   * any constraints that are unsatisfied with details
   * of which rule and row led to the unsatisfied constraint.
   */
  void setPrintUnsatisfiedConstraints();

private:
  // PRIVATE CLASS MEMBERS
  // problem
  const std::vector<std::vector<int>> problem_matrix_;
  std::vector<double> lower_bounds_;
  std::vector<double> upper_bounds_;

  // problem characteristics
  const int variables_count_;
  const int constraints_count_;
  const int inequalities_count_;
  const int equalities_count_;

  // Problem type
  const bool solve_ip_;

  // Vectors to keep track of implied lower and
  // upper bounds during presolve.
  std::vector<double> implied_lower_bounds_;
  std::vector<double> implied_upper_bounds_;

  // feasible solution vector
  std::vector<double> feasible_solution_;

  // vectors to keep track of active rows and columns
  // during presolve
  std::vector<bool> presolve_active_rows_;
  std::vector<bool> presolve_active_columns_;
  int presolve_active_rows_count_;
  int presolve_active_cols_count_;

  // Vector to check we don't apply inequality row
  // singletons presolve twice to the same inequality.
  std::vector<int> inequality_singletons_;

  std::vector<bool> postsolve_active_rows_;
  std::vector<bool> postsolve_active_cols_;

  // Array to keep track of the non-zero active variables in each
  // row and col during presolve.
  std::vector<std::vector<int>> rows_non_zero_variables_;
  std::vector<std::vector<int>> cols_non_zeros_indices_;

  // STRUCTS:
  // struct containing the search function and update
  // state function for a given rule (pointers).

  // (type definition for trying to add postsolve function
  // to the stack)
  // typedef void (Presolve::*member_function)(int, int);

  // struct to keep track of presolve rules applied
  // during presolve.
  struct presolve_log {
    int constraint_index; // constraint number
    int variable_index;   // variable (column) index
    int rule_id;          // presolve rule function to apply in postsolve
    std::vector<int> dependancies;
  };

  // PRIVATE CLASS VARIABLES:
  // stack of presolve structs.
  std::stack<presolve_log> presolve_stack_;

  // PRIVATE METHODS
  /**
   * @brief Gets the indices of the non-zero rows (coefficients)
   * of each column and stores in the
   * instance variable cols_non_zeros_indices;
   * and same for the non-zero variables in each row, storing them
   * in the variable rows_non_zero_variables.
   *
   * @return void
   */
  void getRowsAndColsNonZeros();

  /**
   * @brief Returns the bound with which we will calculate
   * the feasible value of a variable. In general we use the lower
   * bound, but we make sure that if the lower bound is infinity
   * and the upper bound isn't then we return the upper bound.
   * 
   * @param int row_index.
   * @return double: calculation bound.
   */
  double getFeasibleValueCalculationBound(const int row_index);
  
  /**
   * @brief Used when we have an unsatisfied equality in IP
   * postsolve, to check if the equality is a row doubleton.
   * 
   * @param int row_index: Index of corresponding row.
   * @param int col_index: Index of correspinding variable.
   * @return int Inf if not feasible, int feasible value if feasible.
   */
  int getDependancyIndexRowDoubletonIP(
    const int row_index, const int col_index
  );

  /**
   * @brief Used to check if changing the feasible value of the 
   * dependancy variable in a an unsatisfied row doubleton in 
   * IP postsolve leads to all the constraints that contain the 
   * dependancy variable being satisfied.
   * 
   * @param int dependancy_new_feasible_value: New feasible value for the
   * dependancy that we will use to check..
   * @param int dependancy_col_index: Index of dependancy.
   */
  bool checkDependancyIP(
    const int dependancy_new_feasible_value, 
    const int dependancy_col_index
  );

  /**
   * @brief Called in postsolve, when solving ips, 
   * to get the feasible value of a variable once a postsolve 
   * function has simplified the corresponding constraint to an 
   * integer coefficient times the variable in the LHS, 
   * and an integer in the RHS. It checks if the RHS divided by the coefficient is an integer that 
   * satisfies the implied bounds. If not, it applies the integrality
   * check operarions.
   * 
   * @param int variable_coefficient: integer coefficient of the variable.
   * @param int constraint_RHS: integer RHS of constraint.
   * @param int row_index.
   * @return int Inf if not feasible, int feasible value if feasible.
   */
  int getVariableFeasibleValueIP(
    const int row_index, 
    const int col_index, 
    const int variable_coefficient, 
    const double constraint_RHS
  );

  /**
   * @brief Checks if a row is free, that is if it has lower
   * bound of minus infinity and upper bound of infinity.
   *
   * @param int row_index: index of the row.
   * @return Boolean of whether it is free.
   */
  bool checkIsRowFree(const int row_index);

  /**
   * @brief Updates state when a free row is found in presolve.
   *
   * @param int row_index: index of the row.
   * @return void.
   */
  void updateStateFreeRow(const int row_index);

  /**
   * @brief Applies free row postsolve.
   *
   * @param int row_index: index of the row.
   * @return void.
   */
  void applyFreeRowPostsolve(const int row_index);

  /**
   * @brief Updates the state of the problem in presolve given that a
   * row and column singleton has been found. Turns off
   * the row and column and logs the rule into the presolve stack as a
   * row singleton.
   *
   * @param equality row index.
   * @param column index.
   * @return void.
   */
  void updateStateRowAndColSingleton(const int row_index, const int col_index);

  /**
   * @brief Applies postsolve for a row and column singleton. It
   * first finds the feasible value and checks it feasibility. Then, It logs the feasible value,
   * sets the correponding postolve row and col to true,
   *
   * @param equality row index.
   * @param column index.
   * @return void.
   */
  void applyRowAndColSingletonPostsolve(const int row_index,
                                       const int col_index);

  /**
   * @brief Updates the state of the problem in presolve given that a row
   * singleton has been found in an equality. Turns off
   * the row and logs the rule into the presolve stack.
   *
   * @param row_index: equality row index.
   * @param col_index: column index.
   * @return void.
   */
  void updateStateRowSingletonEquality(const int row_index,
                                       const int col_index);

  /**
   * @brief Updates the state of the problem in presolve given that a row
   * singleton has been found in an inequality. It updates the implied
   * bounds on the variable.
   *
   * @param equality row index.
   * @param column index.
   * @return void.
   */
  void updateStateRowSingletonInequality(const int row_index,
                                         const int col_index);

  /**
   * @brief Finds the feasible value of a variable in postsolve
   * when the rule in presolve was row singleton equality. It then updates
   * the feasible solution vector and the state of the problem
   * accordingly.
   *
   * @param equality row index.
   * @param column index.
   * @return void.
   */
  void applyRowSingletonPostsolve(const int row_index);

  /**
   * @brief Checks whether two rows are paralell
   *
   * @param int row_index_1: index of first row.
   * @param int row_index_2: index of second row.
   * @return bool.
   */
  bool checkAreRowsParallel(const int row1_index, const int row2_index);

  /**
   * @brief Given a row, checks if it is parallel to any
   * of the previous rows that are still on, so if it is
   * row i it will check if it is parallel to any row from
   * 0 to i-1. Note that we will never find that it is parallel
   * to two rows j, k in 0 to i-1, because that implies that j and k
   * are also parallel between eachother, hence one of them will
   * already have been turned off. If we find that it there are no
   * rows parallel to i, return -1.
   *
   * @param int row_index: index of row i.
   * @return int, either row in 0 to i-1 parallel to i or -1 if
   * none were found.
   */
  int getParallelRow(const int row_index, const int start);

  /**
   * @brief Given two parallel rows returns a vector with two
   * elements with the index of the row with the larger 
   * coefficients in the first entry and the one with the smaller
   * coefficients in the smaller entry.
   *
   * @param int row: index of row we have rows iteration.
   * @param int parallel_row: index of row parallel to row.
   * @return rows indicies sorted by size.
   */
  std::vector<int> sortParallelRowsBySize(const int row,
                                          const int parallel_row);


  /**
   * @brief Checks if two parallel rows are feasible. Check project
   * report for on how the checks work.
   *
   * @param int small_row_index: index of row with smaller 
   * coefficients.
   * @param int large_to_small_ratio: ratio of large row (larger coefficients)
   * to small row.
   * @param double large_bound_by_ratio: ratio of upper bound of large row
   * divided by large_to_small_ratio.
   * 
   * @return bool.
   */
  bool checkAreParallelRowsFeasible(const int small_row_index,
                                    const int large_to_small_ratio,
                                    const double large_bound_by_ratio);

  /**
   * @brief Updates state of the problem when parallel rows are 
   * found in presolve.
   *
   * @param int small_row_index: index of row with smaller 
   * coefficients.
   * @param int large_row_index: index of row with larger
   * coefficients.
   * @param int large_to_small_ratio: ratio of large row (larger coefficients)
   * to small row.
   * @param double large_bound_by_ratio: ratio of upper bound of large row
   * divided by large_to_small_ratio.
   * 
   * @return void.
   */
  void updateStateParallelRow(const int small_row_index,
                              const int large_row_index,
                              const double large_to_small_ratio,
                              const double large_bound_by_ratio);

  /**
   * @brief Applies postsolve for a row that had been removed
   * from the problem by parallel rows. Removes rule from the 
   * stack and upodates state of the problem accordingly.
   *
   * @param int row_index.
   * @return void.
   */
  void applyParallelRowPostsolve(const int row_index);

  /**
   * @brief Updates the state of the problem in presolve when an
   * empty column has been found. Turns off
   * the column and logs the rule into the presolve stack.
   *
   * @param col_index: column index.
   * @return void.
   */
  void updateStateEmptyCol(const int col_index);

  /**
   * @brief Applies postsolve for empty rows. Feasible value is taken 
   * to be 0 arbitrarily. It then updates the feasible solution
   *  vector and the state of the problem
   * accordingly.
   *
   * @param col_index: column index.
   * @return void.
   */
  void applyEmptyColPostsolve(const int col_index);

  /**
   * @brief Checks if a column is a fixed column by checking
   * the implied bounds on the variable.
   *
   * @param col_index: column index.
   * @return void.
   */
  bool isFixedCol(const int col_index);

  /**
   * @brief Updates state when a fixed col has been found
   * in presolve. Finds the value of the variable and substitutes
   * into the problem matrix, turns off
   * the column and logs the rule into the presolve stack.
   *
   * @param col_index: column index.
   * @return void.
   */
  void updateStateFixedCol(const int col_index);

  /**
   * @brief Carries out the fixed column postsolve procedure.
   * It gets the value of the variable from the implied bounds
   * (which are equal since it is a fixed column) and updates
   * the feasible solution vector. It does not turn the column
   *
   * @param col_index: column index.
   * @return void.
   */
  void applyFixedColPostsolve(const int col_index,
                              const std::vector<int> col_non_zeros);

  /**
   * @brief Checks if a column, which has already been found to be
   * a column singleton, is a free column, i.e if the implied bounds
   * on the variable are minus and plus infinity.
   *
   * @param row_index: of the corresponding row to check.
   * @param col_index: of the corresponding column.
   * @return bool: indicating whether or not column is a free column
   * substitution col.
   */
  bool isFreeColSubstitution(const int row_index, const int col_index);

  /**
   * @brief Updates state of the problem in presolve when a free column
   * substitution column has been found in an equality. Turns off the
   * corresponding row and col and stores the rule in the stack, also storing
   * the dependancy. Note that we differentiate between free column substitution
   * col equality and inequality because the postsolve procedure is different.
   *
   * @param row index.
   * @param column index.
   * @return void.
   */
  void updateStateFreeColSubstitution(const int row_index, const int col_index);

  /**
   * @brief Called in freeColSubstitution postsolve in order to
   * get the sum of all the variables in the row, that are not
   * col_index, times their corresponding coefficients. If the 
   * feasible value for one of the variables has not been found
   * returns infinity.
   *
   * @param row index.
   * @param column index.
   * @return void.
   */
  double getFreeColSubstitutionSumOfDependancies(const int row_index,
                                                 const int col_index);

  /**
   * @brief Finds the feasible value of a variable in postsolve
   * when the rule in presolve was free column substitution. It then
   * updates the feasible solution vector and the state of the problem
   * accordingly.
   *
   * @param int row_index: row index.
   * @param int col_index: col index.
   * @return void.
   */
  void applyFreeColSubstitutionPostsolve(const int row_index,
                                         const int col_index);

  /**
   * @brief Applies the presolve row rules to the problem during
   * a presolve iteration.
   *
   * @return void.
   */
  void applyPresolveRowRules();

  /**
   * @brief Applies the presolve column rules to the problem during
   * a presolve iteration.
   *
   * @return void.
   */
  void applyPresolveColRules();

  /**
   * @brief Checks if the feasible value that has been found for
   * a varibale satisfies the variable's implied bounds.
   *
   * @param variable_index: the index of the variable.
   * @param feasible_value: value found for the variable.
   * @param rule_id: the rule which has been used to find the
   * feasible value.
   * @return bool.
   */
  bool checkVariableImpliedBounds(const int col_index,
                                  const int feasible_value);

  /**
   * @brief Checks if a row is active in postsolve, that is,
   * if we have found feasible values for all its nonzero
   * variables.
   *
   * @param int row_index: index of the row.
   * @return bool.
   */
  bool isRowActivePostsolve(const int row_index);

  /**
   * @brief Checks if a constraint is satisfied in postsolve.
   * Called ones the constraint has been determined as active.
   *
   * @param int row_index: index of the row.
   * @return bool.
   */
  bool checkConstraint(const int row_index, const int rule_id);

  /**
   * @brief Prints the rows and columns that are active in 
   * presolve.
   */
  void printPresolveCurrentState();
};
} // namespace logical_solver

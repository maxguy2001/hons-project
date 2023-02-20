#include <string>
#include <vector>
#include <stack>
#include <limits>
#include <cmath>
#include <iomanip>

namespace logical_solver{

  class Presolve
  {
  public:
  // PUBLIC INSTANCE VARIABLES:
  // problem
  std::vector<std::vector<int>> problem_matrix_;
  std::vector<int> lower_bounds_;
  std::vector<int> upper_bounds_;

  // problem characteristics
  int variables_count_;
  int constraints_count_;
  int inequalities_count_;
  int equalities_count_;

  // Vectors to keep track of implied lower and 
  // upper bounds during presolve.
  std::vector<int> implied_lower_bounds_;
  std::vector<int> implied_upper_bounds_;

  // feasible solution vector
  std::vector<int> feasible_solution;

  // Boolean to check if problem has been reduced to empty.
  bool reduced_to_empty;
  bool infeasible;

  // CONSTRUCTOR
  Presolve(
    const std::vector<std::vector<int>> problem_matrix, 
    const std::vector<int> lower_bounds, 
    const std::vector<int> upper_bounds,
    const int inequalities_count,
    const int equalities_count
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

  private:
  // PRIVATE INSTANCE VARIABLES 
  int kInfinity;

  // vectors to keep track of active rows and columns
  // during presolve
  std::vector<bool> presolve_active_rows_;
  std::vector<bool> presolve_active_columns_;
  int presolve_active_rows_count_;
  int presolve_active_columns_count;

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
    int constraint_index;      // constraint number      
    int variable_index;        // variable (column) index
    int rule_id; // presolve rule function to apply in postsolve
    std::vector<int> dependancies;
  }; 

  // PRIVATE CLASS VARIABLES:
  // stack (vector) of presolve structs.
  std::stack<presolve_log> presolve_stack_;

  // PRIVATE METHODS
  /**
   * @brief Gets the indices of the non-zero columns
   * of each row (non-zero variables) and stores in the 
   * instance variable rows_non_zero_variables.
   * 
   * @return void
   */
  void getRowsNonZeros();

  /**
   * @brief Gets the indices of the non-zero rows (coefficients)
   * of each column and stores in the 
   * instance variable cols_non_zeros_indices.
   * 
   * @return void
   */
  void getColsNonZeros();

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
   * @brief Updates the state of the problem in presolve given that a 
   * redundant variable has been found. Turns off
   * the row and column and logs the rule into the presolve stack as a 
   * row singleton.
   * 
   * @param equality row index.
   * @param column index. 
   * @return void.
   */
  void updateStateRedundantVariable(int row_index, int col_index);

  /**
   * @brief Applies the postsolve steps for a redundant variable. It
   * first finds the feasible value, which we take to be its lower 
   * bound, and checks it feasibility. Then, It logs the feasible value, 
   * sets the correponding postolve row and col to true,
   * 
   * @param equality row index.
   * @param column index. 
   * @return void.
   */
  void applyRedundantVariablePostsolve(int row_index, int col_index);

  /**
   * @brief Updates the state of the problem in presolve given that a row 
   * singleton has been found in an equality. Turns off
   * the row and logs the rule into the presolve stack.
   * 
   * @param row_index: equality row index.
   * @param col_index: column index. 
   * @return void.
   */
  void updateStateRowSingletonEquality(int row_index, int col_index);

  /**
   * @brief Updates the state of the problem in presolve given that a row 
   * singleton has been found in an inequality. It updates the implied 
   * bounds on the variable.
   * 
   * @param equality row index.
   * @param column index. 
   * @return void.
   */
  void updateStateRowSingletonInequality(int row_index, int col_index);

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
  void applyRowSingletonPostsolve(int row_index);

  /**
   * @brief Checks whether two rows are paralell
   * 
   * @param int row_index_1: index of first row.
   * @param int row_index_2: index of second row.
   * @return bool.
   */
  bool checkAreRowsParallel(int row1_index, int row2_index);

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
  int getParallelRow(int row_index, int start);

  std::vector<int> sortParallelRowsBySize(int row, int parallel_row);

  bool checkAreParallelRowsFeasible(
    int small_row_index, 
    int large_to_small_ratio,
    double large_bound_by_ratio
  );

  void updateStateParallelRow(
    int small_row_index, 
    int large_row_index,
    int large_to_small_ratio, 
    double large_bound_by_ratio
  );

  void applyParallelRowPostsolve(
    int row_index, int small_row_index, int small_row_initial_bound
  );

  /**
   * @brief Updates the state of the problem in presolve when an
   * empty column has been found. Turns off
   * the column and logs the rule into the presolve stack.
   * 
   * @param col_index: column index.
   * @return void.
   */
  void updateStateEmptyCol(int col_index);

  /**
   * @brief Finds the feasible value of a variable in postsolve
   * when the rule in presolve was empty column - as per the 
   * presolve stack. Feasible value is taken to be 0 arbitrarily. It then updates
   * the feasible solution vector and the state of the problem 
   * accordingly.
   * 
   * @param col_index: column index.
   * @return void.
   */
  void applyEmptyColPostsolve(int col_index);

  /**
   * @brief Checks if a column is a fixed column by checking 
   * the implied bounds on the variable.
   * 
   * @param col_index: column index.
   * @return void.
   */
  bool isFixedCol(int col_index);

  /**
   * @brief Updates state when a fixed col has been found 
   * in presolve. Finds the value of the variable and substitutes 
   * into the problem matrix, turns off
   * the column and logs the rule into the presolve stack.
   * 
   * @param col_index: column index.
   * @return void.
   */
  void updateStateFixedCol(int col_index);

  /**
   * @brief Carries out the fixed column postsolve procedure.
   * It gets the value of the variable from the implied bounds
   * (which are equal since it is a fixed column) and updates 
   * the feasible solution vector. It does not turn the column 
   * 
   * @param col_index: column index.
   * @return void.
   */
  void applyFixedColPostsolve(int col_index);

  /**
   * @brief Function called in presolve when we know a column 
   * only has one non-zero cofficient, in order to check whether it is
   * a free column substitution column. A free column substitution column happens when there is only
   * one non-zero coefficient in a free column, and it happens
   * in a row where there is only one other variable. Thus,
   * the variable in the column can be expressed in terms of 
   * the other variable in the row. The function checks if the
   * corresponding row only has two non-zero variables by checking the
   * rows_non_zero_variables vector. 
   * 
   * @param row_index: of the corresponding row to check. 
   * @param col_index: of the corresponding column. 
   * @return bool: indicating whether or not column is a free column substitution col.
   */
  bool isFreeColSubstitution(int row_index, int col_index);

  /**
   * @brief Finds the dependancy variable of a free column substitution column,
   * that is the corresponding row that is non-zero when
   * 
   * @param row index.
   * @param column index. 
   * @return int - the index of the dependancy variable.
   */
  int getFreeColSubstitutionDependancy(int row_index, int col_index);

  /**
   * @brief Updates state of the problem in presolve when a free column substitution
   * column has been found in an equality. Turns off the corresponding
   * row and col and stores the rule in the stack, also storing 
   * the dependancy. Note that we differentiate between free column substitution col equality
   * and inequality because the postsolve procedure is different.
   * 
   * @param row index.
   * @param column index. 
   * @return void.
   */
  void updateStateFreeColSubstitution(int row_index, int col_index);

  /**
   * @brief Finds the feasible value of a variable in postsolve
   * when the rule in presolve was free column substitution col - as per the 
   * presolve stack. Feasible value is given by the constant in the lower
   * bounds vector minus the dependancy variable's feasible value 
   * times its coefficient (this will give a feasible value both in 
   * equalities and inequalities).It then updates the feasible solution 
   * vector and the state of the problem 
   * accordingly.
   * 
   * @param int row_index: row index.
   * @param int col_index: col index.
   * @param int dependancy: dependancy variable to work out 
   * feasible solution.
   * @return void.
   */ 
  void applyFreeColSubstitutionPostsolve(
    int row_index, int col_index, int dependancy_index
  );

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
   * @brief Called in postsolve to get the feasible value of 
   * a variable once a postsolve function has simplified the 
   * corresponding constraint to an integer coefficient times 
   * the variable in the LHS, and an integer in the RHS. It checks 
   * if the RHS divided by the coefficient is an integer that 
   * satisfies the implied bounds. If not, if it is an inequality, 
   * tries rounding up to nearest integer and checks if the upper 
   * bound is satisfied.
   * 
   * @param int variable_coefficient: integer coefficient of the variable.
   * @param int constraint_RHS: integer RHS of constraint.
   * @param int row_index.
   * @return int Inf if not feasible, int feasible value if feasible.
   */ 
  int getVariableFeasibleValue(
    int row_index, int col_index, 
    int variable_coefficient, int constraint_RHS
  );

  /**
   * @brief Called from the postsolve functions after getting
   * the variable feasible value with getVariableFeasibleValue.
   * If feasible value is infinity, it sets the instance variable
   * infeasible to true. Else, it updates the feasible_solution vector
   * with the feasible value. It returns false if variable was not 
   * feasible and true if it was.
   * 
   * @param int col_index: the index of the variable.
   * @param int feasible_value: feasible value found for the variable.
   * @return void.
   */
  bool updateStateFeasibleValue(int col_index, int feasible_value);

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
  bool checkVariableImpliedBounds(
    int col_index, int feasible_value
  );

  /**
   * @brief Checks if each constraint is being satisfied correctly
   * as we turn rows and columns back on during postsolve. If a 
   * constraint is not satisfied, returns its index. If they are all
   * satisfied returns -1.
   * 
   * @return void.
   */
  std::vector<int> getUnsatisfiedConstraintsPostsolve();
  
  /**
   * @brief Given a vector of unsatisfied constrains checks if
   * it is empty and if not it raises an exception specifiying 
   * which ones have not been satisfied 
   * 
   * @param unsatisfied_constraints: vector of unsatisfied constraints.
   * @param rule_id: rule after which constraints have not been satisfied.
   * @return void.
   */
  void checkUnsatisfiedConstraintsPostsolve(
    std::vector<int> unsatisfied_constraints,
    int rule_id
  );

  void printPresolveCurrentState();

  
  };
};
  

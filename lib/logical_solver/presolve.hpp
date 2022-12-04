#include <string>
#include <vector>
#include <stack>

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

  private:
  // PRIVATE INSTANCE VARIABLES 
  // vectors to keep track of active rows and columns
  // during presolve
  std::vector<bool> active_rows_;
  std::vector<bool> active_columns_;
  int active_rows_count_;
  // Array to keep track of the non-zero active variables in each
  // row and col during presolve.
  std::vector<std::vector<int>> rows_non_zero_variables_;
  std::vector<std::vector<int>> cols_non_zero_coefficients_;

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
    int variable_index;         // variable (column) index
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
   * instance variable cols_non_zero_coefficients.
   * 
   * @return void
   */
  void getColsNonZeros();

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
   * @brief Updates the state of the problem in presolve given that a row 
   * singleton has been found in an equality. Finds the value of
   * the variable and substitutes into the problem matrix, turns off
   * the row and logs the rule into the presolve stack.
   * 
   * @param equality row index.
   * @param column index. 
   * @return void.
   */
  void updateStateRowSingletonEquality(int row_index, int col_index);

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
  void applyRowSingletonPostsolve(int row_index, int col_index);

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
   * @brief Function called in presolve when we know a column 
   * only has one non-zero cofficient, in order to check whether it is
   * a fixed column. A fixed column happens when there is only
   * one non-zero coefficient in a column, and it happens
   * in a row where there is only one other variable. Thus,
   * the variable in the column can be expressed in terms of 
   * the other variable in the row. The function checks if the
   * corresponding row only has two non-zero variables by checking the
   * rows_non_zero_variables vector. 
   * 
   * @param row_index: of the corresponding row to check. 
   * @return bool: indicating whether or not column is a fixed col.
   */
  bool isFixedCol(int row_index);

  /**
   * @brief Finds the dependancy variable of a fixed column,
   * that is the corresponding row that is non-zero when
   * 
   * @param row index.
   * @param column index. 
   * @return int - the index of the dependancy variable.
   */
  int getFixedColDependancy(int row_index, int col_index);

  /**
   * @brief Updates state of the problem in presolve when a fixed
   * column has been found in an equality. Turns off the corresponding
   * row and col and stores the rule in the stack, also storing 
   * the dependancy. Note that we differentiate between fixed col equality
   * and inequality because the postsolve procedure is different.
   * 
   * @param row index.
   * @param column index. 
   * @return void.
   */
  void updateStateFixedCol(int row_index, int col_index);

  /**
   * @brief Finds the feasible value of a variable in postsolve
   * when the rule in presolve was fixed col - as per the 
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
  void applyFixedColPostsolve(int row_index, int col_index, int dependancy_index);

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
  };
  
};
  

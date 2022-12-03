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
  // Array to keep track of the non-zero variables in each
  // row.
  std::vector<std::vector<int>> rows_non_zero_variables_;

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
   * of a row (non-zero variables).
   * 
   * @param row_index: index of the row.
   * @return vector of indicies of non-zero variables
   */
  std::vector<int> getRowNonZeros(int row_index);

  /**
   * @brief Gets the indices of the non-zero rows
   * of a column (non-zero coefficients of the column
   * variable).
   * 
   * @param column_index: index of the column.
   * @return vector of indicies of non-zero variables
   */
  std::vector<int> getColNonZeros(int col_index);

  /**
   * @brief Updates the state of the problem in presolve given that a row 
   * singleton has been found in an equality. Finds the value of
   * the variable and substitutes into the problem matrix, turns off
   * the row and logs the rule into the presolve stack.
   * 
   * @param equality row index.
   * @param variable index. 
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
   * @param variable index. 
   * @return void.
   */
  void applyRowSingletonEqualityPostsolve(int row_index, int col_index);

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
  

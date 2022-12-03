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
  void apply_presolve();

  /**
   * @brief Applies postsolve to the problem.
   * 
   * @return void.
   */
  void apply_postsolve();

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

  // STRUCTS:
  // struct containing the search function and update 
  // state function for a given rule (pointers).
  typedef void (Presolve::*member_function)(int, int);

  struct rule_functions {
    int (Presolve::*search_function) (std::vector<int>);
    void (Presolve::*update_state_function) (int, int);
  };

  // struct to keep track of presolve rules applied 
  // during presolve.
  struct presolve_log {   
    int constraint_index;      // constraint number      
    int variable_index;         // variable (column) index
    member_function postsolve_function; // presolve rule function to apply in postsolve
  }; 

  // PRIVATE CLASS VARIABLES:
  // stack (vector) of presolve structs.
  std::stack<presolve_log> presolve_stack_;

  // vector of presolve_rule_functions structs for all 
  // the row equality rules.
  std::vector<rule_functions> equality_row_rules; 

  // PRIVATE METHODS
  /**
   * @brief Looks for a row singleton variable in an equality row.
   * 
   * @param equality row.
   * @return Row singleton variable if it was found, -1 if not.
   */
  int search_row_singleton_equality(std::vector<int> row);

  /**
   * @brief Updates the state of the problem given that a row 
   * singleton has been found in an equality. Finds the value of
   * the variable and substitutes into the problem matrix, turns off
   * the row and logs the rule into the presolve stack.
   * 
   * @param equality row index.
   * @param variable index. 
   * @return void.
   */
  void update_state_row_singleton_equality(int row_index, int variable_index);

  /**
   * @brief Finds the feasible value of a variable in postsolve
   * when the rule in presolve was row singleton equality. Updates
   * the feasible solution vector, and the state of the problem 
   * accordingly. 
   * 
   * @param equality row index.
   * @param variable index. 
   * @return void.
   */
  void apply_row_singleton_equality_postsolve(int row_index, int variable_index);
  };

  /**
   * @brief Looks for a row singleton variable in an inequality row.
   * 
   * @param ineequality row.
   * @return Row singleton variable if it was found, -1 if not.
   */
  int search_row_singleton_inequality(std::vector<int> row);
  
}
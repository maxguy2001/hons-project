#include <string>
#include <vector>

namespace logical_solver{

  class Presolve
  {
  public:
  std::vector<std::vector<int>> problem_matrix_;
  std::vector<int> lower_bounds_;
  std::vector<int> upper_bounds_;
  int variables_count_;
  int constraints_count_;

  Presolve(
    const std::vector<std::vector<int>> problem_matrix, 
    const std::vector<int> lower_bounds, 
    const std::vector<int> upper_bounds
  );

  /**
   * @brief Prints the instance LP.
   */
  void printLP();

  private:
  };
  
}
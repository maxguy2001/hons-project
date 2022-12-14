#include <string>
#include <vector>
#include <cstdint>
#include "../core/types.hpp"

namespace utils{

  class PrimalReader
  {
  public:
    PrimalReader();

    /**
     * @brief reads the nth problem in the simplex problem file and writes formatted 
     * contents to class members problem_matrix_.
     * 
     * @param path_to_primal_problem full path to text file containing primal problem
     * @param problem_number number of problem in the file
     */
    void readProblem(const std::string path_to_primal_problem, const core::int_t problem_number);

    std::vector<std::vector<core::int_t>> table_;

  private:

  /**
   * @brief takes input line form text file and converts string to 
   * vector of integers
   * 
   * @param vector_string input string to be formatted
   * @return std::vector<int> formatted vector of ints produced from string input
   */
  std::vector<core::int_t> convertStringToVector(const std::string vector_string);

  };
    
}
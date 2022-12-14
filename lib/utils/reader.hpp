#include <string>
#include <vector>
#include <cstdint>
#include "../core/types.hpp"

namespace utils{

  class Reader
  {
  public:
  Reader();
  /**
   * @brief reads the first problem in the problem file and writes formatted contents
   * to class members problem_matrix_ and cost_vector_.
   * 
   * @param problems_filepath path to problem text file
   */
  void readFirstProblem(const std::string problems_filepath);
  
  /**
   * @brief reads the nth problem in the problem file and writes formatted contents
   * to class members problem_matrix_.
   *  
   * @param problems_filepath path to problem text file
   * @param problem_number number of problem to read in
   */
  void readProblem(const std::string problems_filepath, const core::int_t problem_number);

  std::vector<std::vector<core::int_t>> problem_matrix_;

  const core::int_t kMaxInt = 32765;

  const core::int_t kMinInt = -32765;

  std::vector<core::int_t> upper_bounds_;

  std::vector<core::int_t> lower_bounds_;


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
    
}//namespace utils
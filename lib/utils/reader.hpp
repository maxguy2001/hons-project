#include <string>
#include <vector>
#include <cstdint>
#include <limits>
#include "../core/consts.hpp"

namespace utils{

  class Reader
  {
  public:
  Reader();
  
  /**
   * @brief reads the nth problem in the problem file and writes formatted contents
   * to class members problem_matrix_.
   *  
   * @param problems_filepath path to problem text file
   * @param problem_number number of problem to read in
   */
  void readProblem(const std::string problems_filepath, const int problem_number);

    /**
   * @brief reads the next problem in the problem file and writes formatted contents
   * to class members problem_matrix_ given an open filestream of test problems.
   *  
   * @param problems_filestream the open filestream
   */
  void readNextProblem(std::fstream &problems_filestream);

  std::vector<std::vector<int>> problem_matrix_;

  const int kMaxInt = std::numeric_limits<int>::max(); 

  const int kMinInt = -32765;

  std::vector<int> upper_bounds_;

  std::vector<int> lower_bounds_;

  int num_inequalities_;
  int num_equalities_;


  private:

  /**
   * @brief takes input line form text file and converts string to 
   * vector of integers
   * 
   * @param vector_string input string to be formatted
   * @return std::vector<int> formatted vector of ints produced from string input
   */
  std::vector<int> convertStringToVector(const std::string vector_string);

  std::vector<int> getProblemRowAsIntVector(const std::string problem_row_string);

  std::vector<int> spliceVector(std::vector<int> to_splice, const int range_start, const int range_end);
  };


    
}//namespace utils
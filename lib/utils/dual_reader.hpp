#include "../core/consts.hpp"
#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

namespace utils {

class DualReader {
public:
  DualReader();
  /**
   * @brief reads the first problem in the problem file and writes formatted
   * contents to class members problem_matrix_ and cost_vector_.
   *
   * @param problems_filepath path to problem text file
   */
  void readFirstProblem(const std::string problems_filepath);

  /**
   * @brief reads the next problem in the problem file and writes formatted
   * contents to class members problem_matrix_.
   *
   * @param problem_file filestream to problem file
   */
  void readNextProblem(std::fstream &problem_file);

  // TODO: make these optional?
  std::vector<std::vector<float>> getProblemMatrix();

  std::vector<float> getUpperBounds();

  std::vector<float> getLowerBounds();

private:
  /**
   * @brief takes input line form text file and converts string to
   * vector of integers
   *
   * @param vector_string input string to be formatted
   * @return std::vector<int> formatted vector of ints produced from string
   * input
   */
  std::vector<float> convertStringToVector(const std::string vector_string);

  std::vector<std::vector<float>> problem_matrix_;

  std::vector<float> upper_bounds_;

  std::vector<float> lower_bounds_;
};

} // namespace utils
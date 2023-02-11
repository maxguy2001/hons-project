#include "../core/consts.hpp"
#include <cstdint>
#include <fstream>
#include <optional>
#include <string>
#include <vector>

namespace utils {

class ModifiedPrimalReader {
public:
  ModifiedPrimalReader(std::fstream &filestream);

  /**
   * @brief reads the next problem in the problem file and writes formatted
   * contents to class members problem_matrix_.
   *
   * @param problem_file filestream to problem file
   */
  void readNextProblem(std::fstream &problem_file);

  // TODO: make these optional?
  std::optional<core::InputRows> getNextProblem();

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

  std::fstream &filestream_;

  // counter of filestream problem number, initialised as 0
  uint64_t current_probelm_number_;

  const std::string tilde_ = "~";
};

} // namespace utils
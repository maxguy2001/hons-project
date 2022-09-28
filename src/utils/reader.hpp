#include <string>
#include <string>
#include <vector>
#include <stdint.h>


namespace utils{

  class Reader
  {
  public:
  Reader();

  void readFirstProblem(const std::string problem_filepath);
  void readProblem(const std::string problem_filepath, const int problem_number);
  std::vector<std::vector<int>> problem_matrix_;
  std::vector<int> cost_vector_;

  private:

  std::vector<int> convertStringToVector(const std::string vector_string);



  };
    
}//namespace utils
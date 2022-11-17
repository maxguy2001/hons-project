#include "../lib/core/types.hpp"
#include "../lib/utils/reader.hpp"
#include "../lib/utils/primal_reader.hpp"
#include <vector>
#include <iostream>
#include <fstream>
#include <string>

/// @brief Prints a test case problem. 
/// @param problem_matrix - matrix for the problem constraints.
/// @param lower_bounds - lower bounds for problem constraints.
/// @param upper_bounds - upper bounds for problem constraints.
void printLP(
  const std::vector<std::vector<int>> problem_matrix,
  const std::vector<int> lower_bounds,
  const std::vector<int> upper_bounds
) {
    for (int i = 0; i < problem_matrix.size(); ++i) {
      printf("Row %d \n", i+1);
      // JAJH: [1] should be [0]. Caused segfault on a problem with only one row
      //      for (int j = 0; j < problem_matrix[1].size(); ++j) {
      for (int j = 0; j < problem_matrix[0].size(); ++j) {
        std::cout << problem_matrix[i][j] << std::endl;
      }
      std::cout<<" "<<std::endl;
    }

    std::cout<<" "<<std::endl;
    std::cout<<"Bounds"<<std::endl;;

    for (int i = 0; i < problem_matrix.size(); ++i) {
      std::string upper_bound = std::to_string(upper_bounds[i]);
      if (upper_bound == "32765") {
        upper_bound = "Inf";
      }
      std::cout << lower_bounds.at(i) << ", " << upper_bound << std::endl;
    }
}


int main(){
  std::string test_problems = "/Users/pepe/hons-project/problems/feasibility_testcases.txt";
  utils::Reader reader;

  // Start filestream to pass to reader.
  std::fstream problems_filestream;
  problems_filestream.open(test_problems, std::ios::in);

  std::vector<std::vector<int>>& problem_matrix = reader.problem_matrix_;
  std::vector<int>& upper_bounds = reader.upper_bounds_;
  std::vector<int>& lower_bounds = reader.lower_bounds_;

  for (int i = 0; i < 20; i++){
    std::cout << i << std::endl;
    reader.readNextProblem(problems_filestream);
    printLP(problem_matrix, lower_bounds, upper_bounds);
  }

  problems_filestream.close();

  std::cout << "" << std::endl;
  std::cout << "Printing problem 20 with readProblem" << std::endl;
  reader.readProblem("/Users/pepe/hons-project/problems/feasibility_testcases.txt", 3);
  printLP(problem_matrix, lower_bounds, upper_bounds);

  return 0;
}
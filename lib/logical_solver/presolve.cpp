#include "presolve.hpp"
#include <iostream>

namespace logical_solver{

  Presolve::Presolve(
    const std::vector<std::vector<int>> problem_matrix, 
    const std::vector<int> lower_bounds, 
    const std::vector<int> upper_bounds
  ) {
    // Define problem
    problem_matrix_ = problem_matrix;
    lower_bounds_ = lower_bounds;
    upper_bounds_ = upper_bounds;

    // Get problem attributes
    variables_count_ = problem_matrix[0].size();
    constraints_count_ = problem_matrix.size();
  };

  void Presolve::printLP() {
    for (int i = 0; i < problem_matrix_.size(); ++i) {
      printf("Row %d \n", i+1);

      for (int j = 0; j < variables_count_; ++j) {
        std::cout << problem_matrix_[i][j] << std::endl;
      }
      std::cout<<" "<<std::endl;
    }

    std::cout<<" "<<std::endl;
    std::cout<<"Bounds"<<std::endl;;

    for (int i = 0; i < constraints_count_; ++i) {
      std::string upper_bound = std::to_string(upper_bounds_.at(i));
      if (upper_bound == "32765") {
        upper_bound = "Inf";
      }
      std::cout << lower_bounds_.at(i) << ", " << upper_bound << std::endl;
    }
  };


}
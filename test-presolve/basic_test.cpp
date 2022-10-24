#include "../lib/utils/reader.hpp"
#include "Highs.h"
#include <vector>
#include <iostream>
#include <string>

int main() {
  utils::Reader reader_;
  reader_.readProblem("/Users/pepe/hons-project/problems/feasibility_testcases.txt", 1);
  std::vector<std::vector<int>> mat = reader_.problem_matrix_;
  std::vector<int> max = reader_.upper_bounds_;
  std::vector<int> min = reader_.lower_bounds_;


  for(int i =0; i < mat.size(); ++i){
    for(int j = 0; j < mat[1].size(); ++j){
      std::cout << mat[i][j] << std::endl;
    }
  }

  for(int i = 0; i < max.size(); ++i){
    std::cout << max.at(i) << ", " << min.at(i) << std::endl;
  }


  return 0;
}
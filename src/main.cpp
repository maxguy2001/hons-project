#include "utils/reader.hpp"
#include <vector>
#include <iostream>

int main(){
  utils::Reader reader_;
  reader_.readProblem("/home/maxguy/projects/hons/hons-project/problems/feasibility_testcases.txt", 2);
  std::vector<std::vector<int16_t>> mat = reader_.problem_matrix_;
  std::vector<int16_t> vec = reader_.cost_vector_;

/*
  for(int i = 0; i < vec.size(); ++i){
    std::cout << vec.at(i) << std::endl;
  }
*/


  for(int i =0; i < mat.size(); ++i){
    for(int j = 0; j < mat[1].size(); ++j){
      std::cout << mat[i][j] << std::endl;
    }
  }


  return 0;
}
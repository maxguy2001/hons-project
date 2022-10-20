#include "../lib/utils/reader.hpp"
#include "../lib/utils/primal_reader.hpp"
#include <vector>
#include <iostream>

int main(){
  utils::PrimalReader reader_;
  //reader_.readFirstProblem("/home/maxguy/projects/hons/hons-project/problems/feasibility_testcases.txt");

  reader_.readProblem("/home/maxguy/projects/hons/hons-project/problems/feasibility_testcases.txt", 3);
  std::vector<std::vector<int16_t>> mat = reader_.problem_matrix_;
  std::vector<int16_t> max = reader_.upper_bounds_;
  std::vector<int16_t> min = reader_.lower_bounds_;


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
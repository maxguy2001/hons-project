#include "../lib/utils/reader.hpp"
#include "../lib/utils/primal_reader.hpp"
#include <vector>
#include <iostream>

int main(){
  utils::PrimalReader reader_;
  //reader_.readFirstProblem("/home/maxguy/projects/hons/hons-project/problems/feasibility_testcases.txt");
  reader_.readProblem("/home/maxguy/projects/hons/hons-project/problems/primal_problems.txt", 1);
  std::vector<std::vector<int16_t>> mat = reader_.table_;


  for(int i =0; i < mat.size(); ++i){
    for(int j = 0; j < mat[1].size(); ++j){
      std::cout << mat[i][j] << std::endl;
    }
  }


  return 0;
}
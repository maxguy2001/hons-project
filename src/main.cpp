#include "../lib/core/types.hpp"
#include "../lib/utils/reader.hpp"
#include "../lib/utils/primal_reader.hpp"
#include <vector>
#include <iostream>

int main(){
  utils::PrimalReader reader_;
  //reader_.readFirstProblem("/home/maxguy/projects/hons/hons-project/problems/feasibility_testcases.txt");

  reader_.readProblem("/home/maxguy/projects/hons/hons-project/problems/primal_problems.txt", 2);

  std::vector<std::vector<int>> table = reader_.table_;
  std::vector<int> basis = reader_.basis_;


  for(int i =0; i < table.size(); ++i){
    for(int j = 0; j < table[1].size(); ++j){
      std::cout << table[i][j] << std::endl;
    }
  }

  for(int i = 0; i < basis.size(); ++i){
    std::cout << basis.at(i) << std::endl;
  }

  return 0;
}
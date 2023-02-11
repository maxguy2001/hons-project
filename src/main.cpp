#include "../lib/primal_simplex/main.hpp"
#include "../lib/core/consts.hpp"
#include "../lib/utils/modified_primal_reader.hpp"
#include "../lib/utils/primal_reader.hpp"
#include "../lib/utils/reformatter.hpp"
#include <cmath>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

int main() {

  const std::string pp = "/home/maxguy/projects/hons/hons-project/problems/"
                         "feasibility_testcases.txt";
  std::fstream filestream;
  filestream.open(pp, std::ios::in);
  utils::ModifiedPrimalReader reader_(filestream);

  auto problem = reader_.getNextProblem();

  utils::Reformatter rf_;
  std::vector<std::vector<float>> rf_prob = rf_.reformatProblem(*problem);

  for (std::size_t i = 0; i < rf_prob.size(); ++i) {
    for (std::size_t j = 0; j < rf_prob.at(0).size(); ++j) {
      std::cout << rf_prob.at(i).at(j) << "  ";
    }
    std::cout << std::endl;
  }

  return 0;
}
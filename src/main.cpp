#include "../lib/primal_simplex/main.hpp"
#include "../lib/core/consts.hpp"
#include "../lib/revised_primal_solver/simplex.hpp"
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
  core::FormattedProblem rf_prob = rf_.reformatProblem(*problem);

  revised_primal_simplex::RevisedPrimalSimplex solver_;
  solver_.setProblem(rf_prob.problem_matrix);
  solver_.setBasis(rf_prob.basic_variables);
  auto solution_row = solver_.solveProblem();

  return 0;
}

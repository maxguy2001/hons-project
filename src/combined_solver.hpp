#include "../lib/core/consts.hpp"
#include "../lib/solvers/logical_solver/presolve.hpp"
#include "../lib/solvers/primal_bland/bland_simplex.hpp"
#include "../lib/solvers/primal_simplex/main.hpp"
#include "../lib/solvers/revised_primal_solver/simplex.hpp"
#include "../lib/utils/modified_primal_reader.hpp"
//#include "../lib/utils/primal_reader.hpp"
#include "../lib/utils/logical_reformatter.hpp"
#include "../lib/utils/primal_reformatter.hpp"
#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

class CombinedRun {
public:
  // empty constructor
  CombinedRun() {}

  void runSolver(const std::string problems_filepath);
};
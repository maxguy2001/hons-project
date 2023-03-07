#include "../lib/core/consts.hpp"
#include "../lib/solvers/primal_bland/bland_simplex.hpp"
#include "../lib/solvers/primal_simplex/main.hpp"
#include "../lib/solvers/revised_primal_solver/simplex.hpp"
#include "../lib/utils/modified_primal_reader.hpp"
#include "../lib/utils/primal_reader.hpp"
#include "../lib/utils/reformatter.hpp"
#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

class PrimalRun {
public:
  // empty constructor
  PrimalRun() {}

  void runPrimalSolver();
  void printProblem(const core::InputRows input_rows);
};
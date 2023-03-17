#include "../lib/core/consts.hpp"
#include "../lib/core/types.hpp"
#include "../lib/solvers/primal_bland/bland_simplex.hpp"
#include "../lib/utils/primal_reformatter.hpp"
#include "../lib/utils/reader.hpp"
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

  void runPrimalSolver(const std::string problems_filepath);
  void printProblem(const core::InputRows input_rows);
};
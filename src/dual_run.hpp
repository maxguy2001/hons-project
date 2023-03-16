#include "../lib/core/consts.hpp"
#include "../lib/core/types.hpp"
#include "../lib/solvers/dual_simplex/dual_blands.hpp"
#include "../lib/utils/dual_reformatter.hpp"
#include "../lib/utils/reader.hpp"
#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

class DualRun {
public:
  // empty constructor
  DualRun() {}

  void runDualSolver();
  void printProblem(const core::InputRows input_rows);
  void temp();
};
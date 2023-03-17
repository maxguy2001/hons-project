#include "../lib/deprecated/presolve_reader.hpp"
#include "../lib/solvers/logical_solver/presolve.hpp"
#include <chrono>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

class LogicalSolverRun {
public:
  // empty constructor
  LogicalSolverRun() {}

  void testOnTestProblem();
  void testOnMultipleProblems(const int problems_count,
                              const std::string problems_filepath);
  void testOnFullTestSet(const std::string problems_filepath);
};
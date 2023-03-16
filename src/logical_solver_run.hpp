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
  void testOnSingleProblem(const int problem_number);
  void testOnMultipleProblems(const int problems_count);
  void testOnFullTestSet();
};
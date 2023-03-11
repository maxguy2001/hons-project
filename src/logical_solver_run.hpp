#include "../lib/solvers/logical_solver/presolve.hpp"
#include "../lib/utils/reader.hpp"
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <chrono>


class LogicalSolverRun {
public:
  // empty constructor
  LogicalSolverRun() {}

  void testOnTestProblem();
  void testOnSingleProblem(const int problem_number);
  void testOnMultipleProblems(const int problems_count);
  void testOnFullTestSet();
};
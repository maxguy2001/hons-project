#include "main.hpp"
#include <string>

int main() {

  const std::string user = "max";

  std::string problem_path;
  if (user == "josep") {
    problem_path =
        "/Users/pepe/hons-project/problems/feasibility_testcases.txt";
  } else if (user == "max") {
    problem_path = "/home/maxguy/projects/hons/hons-project/"
                   "problems/feasibility_testcases.txt";
  } else {
    std::cout << "Error in main.cpp: no file path given" << std::endl;
  }

  CombinedRun run_;
  run_.runSolver(problem_path);

  // DualRun run_;
  // run_.runDualSolver();

  // LogicalSolverRun run_;
  // run_.testOnFullTestSet();

  return 0;
}
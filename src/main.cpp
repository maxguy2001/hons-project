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
  } else if (user == "other") {
    std::cout << "Input full local path to problem file" << std::endl;
    std::cin >> problem_path;
  } else {
    std::cout << "Error in main.cpp: no file path given" << std::endl;
  }

  if (problem_path.length() == 0) {
    std::cout << "No path to problem file given" << std::endl;
    return 0;
  }

  CombinedRun run_;
  run_.runSolver(problem_path);

  // DualRun run_;
  // run_.runDualSolver(problem_path);

  // LogicalSolverRun run_;
  // run_.testOnFullTestSet();

  return 0;
}
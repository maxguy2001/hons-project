#include "main.hpp"
#include <iostream>
#include <string>

int main() {

  const std::string user = "josep";

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

  // LogicalSolverRun run_;
  // run_.testOnFullTestSet();

  return 0;
}
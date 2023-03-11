#include "logical_solver_run.hpp"
#include "primal_run.hpp"


int main(){
  LogicalSolverRun logical_run_;
  logical_run_.testOnFullTestSet();
  // PrimalRun run_;
  // run_.runPrimalSolver();

  return 0;
}
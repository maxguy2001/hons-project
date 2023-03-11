#include <core/consts.hpp>

namespace logical_solver {

class LogicalSolver : public core::ISolver {
public:
  LogicalSolver();
  void solveProblem();

private:
};

LogicalSolver::LogicalSolver() {}

} // namespace logical_solver
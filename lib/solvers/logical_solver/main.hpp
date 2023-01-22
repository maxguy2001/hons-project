#include "../../core/consts.hpp"

namespace logical_solver {

class LogicalSolver : public core::ISolver {
public:
  LogicalSolver();
  void setProblem(const std::vector<std::vector<float>> table) = 0;
  void setBasis(const std::vector<int> basis) = 0;
  void solveProblem() = 0;

private:
};

LogicalSolver::LogicalSolver() {}

} // namespace logical_solver
#include "../../core/consts.hpp"

namespace dual_simplex {

class DualSimplex : public core::ISolver {

public:
  DualSimplex();
  void setProblem(const std::vector<std::vector<float>> table) = 0;
  void setBasis(const std::vector<int> basis) = 0;
  void solveProblem() = 0;

private:
};

DualSimplex::DualSimplex() {}

} // namespace dual_simplex
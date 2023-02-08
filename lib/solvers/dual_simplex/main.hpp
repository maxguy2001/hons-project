#include "../../core/consts.hpp"

namespace dual_simplex {

class DualSimplex : public core::ISolver {

public:
  DualSimplex();
  void solveProblem();

private:
};

DualSimplex::DualSimplex() {}

} // namespace dual_simplex
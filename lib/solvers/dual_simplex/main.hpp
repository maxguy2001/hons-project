#include <core/consts.hpp>
#include <core/types.hpp>

namespace dual_simplex {

class DualSimplex : public core::ISolver {

public:
  DualSimplex();
  void solveProblem();

private:
};

DualSimplex::DualSimplex() {}

} // namespace dual_simplex
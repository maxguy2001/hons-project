#pragma once

#include <limits>
#include <vector>

namespace core {

// define value for infinity
const int kIntInfinity = std::numeric_limits<int>::max();
const float kFloatInfinity = std::numeric_limits<float>::max();
const float kEpsilon = 1e-14;
const int kMaxIterations = 1000;

class ISolver {
public:
  // TODOLater: consider implementing these?
  // virtual void setProblem(const std::vector<std::vector<float>> table) = 0;
  // virtual void setBasis(const std::vector<int> basis) = 0;
  virtual void solveProblem() = 0;
};

} // namespace core

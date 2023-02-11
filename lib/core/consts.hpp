#pragma once

#include <limits>
#include <vector>

namespace core {

struct InputRows {
  std::vector<std::vector<float>> equality_rows;
  std::vector<std::vector<float>> inequality_rows;
};

// define value for infinity
const int kIntInfinity = std::numeric_limits<int>::max();
const float kFloatInfinity = std::numeric_limits<float>::max();
const float kEpsilon = 1e-14;
const int kMaxIterations = 1000;

} // namespace core

#pragma once

#include <limits>
#include <vector>

namespace core {

struct InputRows {
  std::vector<std::vector<float>> equality_rows;
  std::vector<std::vector<float>> inequality_rows;
};

struct FormattedProblem {
  std::vector<std::vector<float>> problem_matrix;
  std::vector<int> basic_variables;
};

// define value for infinity
const int kIntInfinity = std::numeric_limits<int>::max();
const float kFloatInfinity = std::numeric_limits<float>::max();
const float kEpsilon = 1e-14;
const int kMaxIterations = 100;

} // namespace core

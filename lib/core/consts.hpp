#pragma once

#include <limits>
#include <vector>

namespace core {

// define value for infinity
const int kIntInfinity = std::numeric_limits<int>::max();
const double kFloatInfinity = std::numeric_limits<double>::max();
const float kEpsilon = 1e-14;
const int kMaxIterations = 1000;

} // namespace core

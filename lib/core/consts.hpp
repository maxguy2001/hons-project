#pragma once

#include <limits>
#include <vector>

namespace core {

enum class PresolveRulesIds {
  freeRowId = 0,
  singletonVariableId,
  rowSingletonId,
  parallelRowId,
  emptyColId,
  fixedColId,
  freeColSubsId
};

struct InputRows {
  std::vector<std::vector<float>> equality_rows;
  std::vector<std::vector<float>> inequality_rows;
  int num_variables;
};

struct FormattedPrimalProblem {
  std::vector<std::vector<float>> problem_matrix;
  std::vector<int> basic_variables;
};

struct FormattedLogicalProblem {
  std::vector<std::vector<int>> problem_matrix;
  std::vector<int> lower_bounds;
  std::vector<int> upper_bounds;
};

enum class SolveStatus { kInfeasible = 0, kFeasible, kError, kDidntConverge };

// define value for infinity
const int kIntInfinity = std::numeric_limits<int>::max();
const double kFloatInfinity = std::numeric_limits<double>::max();
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

#pragma once

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

struct FormattedDualProblem {
  std::vector<std::vector<float>> problem_matrix;
  std::vector<int> basic_variables;
};

struct FormattedLogicalProblem {
  std::vector<std::vector<int>> problem_matrix;
  std::vector<int> lower_bounds;
  std::vector<int> upper_bounds;
};

enum class SolveStatus { kInfeasible = 0, kFeasible, kError, kDidntConverge };

class ISolver {
public:
  virtual void solveProblem() = 0;
};

} // namespace core
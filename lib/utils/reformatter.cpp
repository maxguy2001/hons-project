#include "reformatter.hpp"

namespace utils {

Reformatter::Reformatter() {}

std::vector<std::vector<float>>
Reformatter::reformatProblem(const core::InputRows input_rows) {
  const std::vector<float> primal_bounds = getPrimalBounds(input_rows);
  const std::vector<std::vector<float>> primal_table =
      getPrimalTable(input_rows);
  const std::vector<float> dual_objective_row =
      getDualObjectiveRow(primal_bounds, primal_table);
  const std::vector<std::vector<float>> dual_table =
      getFullDualTable(primal_table, dual_objective_row);
  return dual_table;
}

std::vector<float>
Reformatter::getPrimalBounds(const core::InputRows input_rows) {
  // Empty return to build for now
  std::vector<float> x;
  return x;
}

std::vector<std::vector<float>>
Reformatter::getPrimalTable(const core::InputRows input_rows) {
  // Empty return to build for now
  std::vector<std::vector<float>> x;
  return x;
}

std::vector<float> Reformatter::getDualObjectiveRow(
    const std::vector<float> primal_bounds,
    const std::vector<std::vector<float>> primal_table) {
  // Empty return to build for now
  std::vector<float> x;
  return x;
}

std::vector<std::vector<float>> Reformatter::getFullDualTable(
    const std::vector<std::vector<float>> primal_table,
    const std::vector<float> objective_row) {
  // Empty return to build for now
  std::vector<std::vector<float>> x;
  return x;
}

} // namespace utils
#include <cstdint>
#include <iostream>
#include <vector>

#include "../core/consts.hpp"

namespace utils {

class Reformatter {
public:
  Reformatter();

  core::FormattedProblem reformatProblem(const core::InputRows input_rows);

private:
  std::vector<float> getPrimalBounds(const core::InputRows input_rows);

  std::vector<std::vector<float>>
  getPrimalTable(const core::InputRows input_rows);

  std::vector<float>
  getDualObjectiveRow(const std::vector<float> primal_bounds,
                      const std::vector<std::vector<float>> primal_table);

  core::FormattedProblem
  getFullDualTable(const std::vector<std::vector<float>> primal_table,
                   const std::vector<float> objective_row);

  uint16_t num_inequality_rows_;
  uint16_t num_equality_rows_;
};

} // namespace utils
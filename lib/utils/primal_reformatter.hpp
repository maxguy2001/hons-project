#include <cstdint>
#include <iostream>
#include <vector>

#include "../core/consts.hpp"
#include "../core/types.hpp"

namespace utils {

class PrimalReformatter {
public:
  PrimalReformatter();

  core::FormattedPrimalProblem
  reformatProblem(const core::InputRows input_rows);

private:
  std::vector<float> getPrimalBounds(const core::InputRows input_rows);

  std::vector<std::vector<float>>
  getPrimalTable(const core::InputRows input_rows);

  std::vector<float>
  getDualObjectiveRow(const std::vector<float> primal_bounds,
                      const std::vector<std::vector<float>> primal_table);

  core::FormattedPrimalProblem
  getFullDualTable(const std::vector<std::vector<float>> primal_table,
                   const std::vector<float> objective_row);

  uint16_t num_inequality_rows_;
  uint16_t num_equality_rows_;
};

} // namespace utils
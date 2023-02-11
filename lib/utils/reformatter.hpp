#include <cstdint>
#include <vector>

#include "../core/consts.hpp"

namespace utils {

class Reformatter {
public:
  Reformatter();

  std::vector<std::vector<float>>
  reformatProblem(const core::InputRows input_rows);

private:
  std::vector<float> getPrimalBounds(const core::InputRows input_rows);

  std::vector<std::vector<float>>
  getPrimalTable(const core::InputRows input_rows);

  std::vector<float>
  getDualObjectiveRow(const std::vector<float> primal_bounds,
                      const std::vector<std::vector<float>> primal_table);

  std::vector<std::vector<float>>
  getFullDualTable(const std::vector<std::vector<float>> primal_table,
                   const std::vector<float> objective_row);

  uint16_t num_inequality_rows_;
  uint16_t num_equality_rows_;
};

} // namespace utils
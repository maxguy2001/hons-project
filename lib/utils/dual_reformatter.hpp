#include "../core/consts.hpp"
#include <cstdint>
#include <vector>

namespace utils {

class DualRefomatter {
public:
  DualRefomatter();

  core::FormattedDualProblem reformatProblem(const core::InputRows input_rows);

private:
  std::vector<float> getDualBounds(const core::InputRows input_rows);

  std::vector<std::vector<float>>
  getDualTable(const core::InputRows input_rows);

  std::vector<float>
  getObjectiveRow(const std::vector<float> dual_bounds,
                  const std::vector<std::vector<float>> dual_table);

  core::FormattedDualProblem
  getFullTable(const std::vector<std::vector<float>> dual_table,
               const std::vector<float> objective_row);
};
} // namespace utils
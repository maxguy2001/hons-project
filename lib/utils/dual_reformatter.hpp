#include "../core/consts.hpp"
#include <cstdint>
#include <vector>

namespace utils {

class DualRefomatter {
public:
  DualRefomatter();

  core::FormattedDualProblem reformatProblem(const core::InputRows input_rows);

private:
  std::vector<float> getBounds(const core::InputRows input_rows);

  std::vector<std::vector<float>> getTable(const core::InputRows input_rows);

  core::FormattedDualProblem
  getFullTable(const std::vector<std::vector<float>> input_table,
               const std::vector<float> bounds);
};
} // namespace utils
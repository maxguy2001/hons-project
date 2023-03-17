#include <cstdint>
#include <iostream>
#include <vector>

#include "../core/consts.hpp"
#include "../core/types.hpp"

namespace utils {

class DualReformatter {
public:
  DualReformatter();

  core::FormattedDualProblem reformatProblem(const core::InputRows input_rows);

  // private:
  std::vector<float> getBounds(const core::InputRows input_rows);

  std::vector<std::vector<float>> getTable(const core::InputRows input_rows);

  std::vector<std::vector<float>>
  constructFullTable(const std::vector<std::vector<float>> primal_table,
                     const std::vector<float> bounds);

  std::vector<int>
  getBasicVariables(const std::vector<std::vector<float>> full_table,
                    const std::vector<float> bounds);
};

} // namespace utils
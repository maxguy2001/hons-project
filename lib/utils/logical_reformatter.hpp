#include <cstdint>
#include <iostream>
#include <vector>

#include "../core/consts.hpp"

namespace utils {

class LogicalReformatter {
public:
  LogicalReformatter();

  core::FormattedLogicalProblem
  reformatProblem(const core::InputRows input_rows);

private:
  std::vector<std::vector<int>>
  getProblemMatrix(const core::InputRows input_rows);
  std::vector<int> getLowerBounds(const core::InputRows input_rows);
  std::vector<int> getUpperBounds(const core::InputRows input_rows,
                                  const std::vector<int> lower_bounds);
};

} // namespace utils
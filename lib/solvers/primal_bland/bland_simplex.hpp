#include <cstdint>
#include <optional>
#include <vector>

#include "../../core/consts.hpp"
#include "../../core/types.hpp"

namespace solvers::bland_simplex {

class BlandPrimalSimplex {
public:
  BlandPrimalSimplex();

  /**
   * @brief Set the table class member
   *
   * @param table simplex table
   */
  void setProblem(const std::vector<std::vector<float>> table);

  /**
   * @brief Set the initial simplex basis
   *
   * @param basis initial basis
   */
  void setBasis(const std::vector<int> basis);

  /**
   * @brief solve the given problem
   *
   */
  core::SolveStatus solveProblem(const bool run_verbose,
                                 const core::InputRows original_problem);

  void printSolution();

private:
  // simplex table
  std::vector<std::vector<float>> table_;

  std::vector<float> solution_;

  // simplex basis. Order of basis must be preserved!
  std::vector<int> basis_;

  /**
   * @brief returns a column of the table as a vector. Useful for iterating over
   *
   * @param column_index index of column to be extracted from table
   * @return std::vector<float>
   */
  std::vector<float> extractColumnFromTable(const int column_index);

  /**
   * @brief returns the index of the smallest nonbasic entry in the objective
   * function (first row of table_)
   *
   * @return int
   */
  int getPivotColumnIndex();

  /**
   * @brief returns index of pivot row in table_ based on the minimum value
   * found during ratio testing
   *
   * @param pivot_column_index index found previously in getPivotColumnIndex()
   * funtion
   * @return int
   */
  int getPivotRowIndex(const int pivot_column_index);

  /**
   * @brief switch basis_ based on pivot row and column row
   *
   * @param pivot_row_index found in getPivotRowIndex()
   * @param pivot_column_index found in getPivotColumnIndex()
   */
  bool switchBasis(const int pivot_row_index, const int pivot_column_index);

  /**
   * @brief constructs new table_ object, completing row operations to reduce
   * required variable to basic
   *
   * @param pivot_row_index
   * @param pivot_column_index
   */
  void constructNewTable(const int pivot_row_index,
                         const int pivot_column_index);

  /**
   * @brief checks if solution is optimal yet.
   * Solution is optimal if all elements of objective funciton (first row of
   * table_) are all positive.
   *
   * @return true
   * @return false
   */
  bool checkOptimality();

  core::SolveStatus verifySolution(core::InputRows original_problem,
                                   std::vector<float> solution_row);

  void printObjectiveRow();
};

} // namespace solvers::bland_simplex

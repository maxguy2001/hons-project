#include <vector>
#include <cstdint>

namespace primal_simplex{

  class PrimalSimplex
  {
  public:
    PrimalSimplex();

    void setProblem(const std::vector<std::vector<float>> table);

    void setBasis(const std::vector<int> basis);

    void solveProblem();

    void printSolution();

  private:

    std::vector<std::vector<float>> table_;

    std::vector<int> basis_;

    std::vector<float> extractColumnFromTable(const int column_index);

    int getPivotColumnIndex();

    int getPivotRowIndex(const int pivot_column_index);

    void switchBasis(const int pivot_row_index, const int pivot_column_index);

    void constructNewTable(const int pivot_row_index, const int pivot_column_index);

    bool checkOptimality();

  };
    
}
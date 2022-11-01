#include <vector>

namespace primal_simplex{

  class PrimalSimplex
  {
  public:
    PrimalSimplex();

    void setProblem(const std::vector<std::vector<int>>);

    void setBasis(const std::vector<int>);

    void solveProblem();

    void printSolution();

  private:

    int getPivotColumnIndex();

    int getPivotRowIndex();

    void switchBasis();

    int gcd(const int a, const int b);

    int lcm(const int a, const int b);

    void constructNewTable();

    bool checkOptimality();

  };
  
  PrimalSimplex::PrimalSimplex(){}
  
}
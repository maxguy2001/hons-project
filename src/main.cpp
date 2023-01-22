#include "../lib/primal_simplex/main.hpp"
#include "../lib/core/consts.hpp"
#include "../lib/utils/primal_reader.hpp"
#include "../lib/utils/reader.hpp"
#include <iostream>
#include <vector>

int main() {
  utils::PrimalReader reader_;
  // reader_.readFirstProblem("/home/maxguy/projects/hons/hons-project/problems/feasibility_testcases.txt");

  reader_.readProblem(
      "/home/maxguy/projects/hons/hons-project/problems/primal_problems.txt",
      1);

  std::vector<std::vector<int>> table = reader_.table_;
  std::vector<int> basis = reader_.basis_;

  std::vector<std::vector<float>> table_float;
  std::vector<float> temp_vec;
  for (int i = 0; i < table.size(); ++i) {
    for (int j = 0; j < table[0].size(); ++j) {
      temp_vec.push_back(static_cast<float>(table.at(i).at(j)));
    }
    table_float.push_back(temp_vec);
    temp_vec.clear();
  }

  primal_simplex::PrimalSimplex solver_;
  solver_.setProblem(table_float);
  solver_.setBasis(basis);
  solver_.solveProblem();

  /*
   // iter 0
   int pivot_column_index = solver_.getPivotColumnIndex();
   int pivot_row_index = solver_.getPivotRowIndex(pivot_column_index);
   solver_.switchBasis(pivot_row_index, pivot_column_index);
   solver_.constructNewTable(pivot_row_index, pivot_column_index);

   if (solver_.checkOptimality()) {
     std::cout << "optimal solition found";
   }

   // iter 1
   pivot_column_index = solver_.getPivotColumnIndex();
   // pivot row index wrong!
   pivot_row_index = solver_.getPivotRowIndex(pivot_column_index);
   solver_.switchBasis(pivot_row_index, pivot_column_index);
   solver_.constructNewTable(pivot_row_index, pivot_column_index);
   // solver_.printTable();
   // solver_.printBasis();
   std::cout << pivot_column_index << " " << pivot_row_index << std::endl;

   if (solver_.checkOptimality()) {
     std::cout << "optimal solition found";
   }


  // iter 2
  pivot_column_index = solver_.getPivotColumnIndex();
  pivot_row_index = solver_.getPivotRowIndex(pivot_column_index);
  solver_.switchBasis(pivot_row_index, pivot_column_index);
  solver_.constructNewTable(pivot_row_index, pivot_column_index);
  // solver_.printTable();
  // solver_.printBasis();

  if (solver_.checkOptimality()) {
    std::cout << "optimal solition found" << std::endl;
  }

  solver_.printSolution();
  */

  return 0;
}
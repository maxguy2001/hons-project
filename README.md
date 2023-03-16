# hons-project
Repository for 4th year mathematics honors project at University of Edinburgh.

## Objective

The aim of this project is to build a fast and relaible solver capable of solving linear feasibility (and more genrally, LP) problems. Further details are given in the final report on this project.

## Approach

The approach was to build a combined solver which uses both presolve and simplex methods to get the fastest and most robust solver possible. These seperate solvers are then used in conjunction with presolve being tried first since it is faster but will only solve a subset of the problems. If the presolve method does not yield a solution, the slower but more robust simplex solver is used.


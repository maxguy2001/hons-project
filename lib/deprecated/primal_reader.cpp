#include "primal_reader.hpp"

#include <fstream>
#include <iostream>

namespace deprecated {

PrimalReader::PrimalReader() {}

void PrimalReader::readProblem(const std::string path_to_primal_problem,
                               const int problem_num) {
  // clear contents of vectors before redefining
  table_.clear();
  basis_.clear();

  // initialise filestream and open file
  std::fstream newfile;
  newfile.open(path_to_primal_problem, std::ios::in);
  // check file is open
  if (!newfile.is_open()) {
    std::cout << "ERROR: Unable to open file" << std::endl;
  }

  // find location in file we want to look at (nth problem)
  int current_problem_location = 0;
  std::string tempstring;
  std::string tilde = "~";
  while (current_problem_location != problem_num - 1) {
    std::getline(newfile, tempstring);
    if (tempstring.find(tilde) != std::string::npos) {
      current_problem_location += 1;
    }
  }

  // find how many rows to read in and expected length
  std::string num_rows_string;
  std::string num_vars_string;

  std::getline(newfile, num_rows_string);
  std::getline(newfile, num_vars_string);

  const int num_rows = atoi(num_rows_string.c_str());
  const int num_vars = atoi(num_vars_string.c_str());

  // define temp string and vector for holding
  std::string temp_string;
  std::vector<int> temp_vec;
  for (size_t i = 0; i < num_rows; ++i) {
    std::getline(newfile, temp_string);
    temp_vec = convertStringToVector(temp_string);

    // check length of vector
    if (temp_vec.size() != num_vars) {
      std::cout << "Error: Length of vector is not equal to expected value"
                << std::endl;
    }

    // add vector to table
    table_.push_back(temp_vec);
    temp_vec.clear();
  }

  // get expected length of basis vector
  std::string length_basis_string;
  std::getline(newfile, length_basis_string);
  const int length_basis = std::atoi(length_basis_string.c_str());

  // get basis vector
  std::getline(newfile, temp_string);
  const std::vector<int> basis = convertStringToVector(temp_string);

  // assign basis to class member
  for (size_t i = 0; i < length_basis; ++i) {
    basis_.push_back(basis.at(i));
  }

  newfile.close();
}

std::vector<int>
PrimalReader::convertStringToVector(const std::string vector_string) {

  std::string tempstring;
  std::vector<std::string> stringvec;

  // convert single string to vector of strings with space character as
  // delimiter
  for (size_t i = 0; i < vector_string.length(); ++i) {
    tempstring.push_back(vector_string.at(i));
    if (vector_string.at(i) == ' ') {
      stringvec.push_back(tempstring);
      tempstring.clear();
    }
    if (i == vector_string.length() - 1) {
      stringvec.push_back(tempstring);
    }
  }

  // initialise variables
  std::vector<int> rowvector;
  int temp_int;
  std::string space = " ";

  // if string in vector is not empty or a space, convert to int and add to
  // return vector
  for (size_t i = 0; i < stringvec.size(); ++i) {
    tempstring = stringvec.at(i);
    if (tempstring.compare(space) != 0 && !tempstring.empty()) {
      temp_int = atoi(tempstring.c_str());
      rowvector.push_back(temp_int);
    }
  }
  return rowvector;
}

} // namespace deprecated
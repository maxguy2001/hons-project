#include "reader.hpp"
#include <iostream>

namespace utils {
// Must be initialised with OPEN filestream
ModifiedPrimalReader::ModifiedPrimalReader(std::fstream &filestream)
    : filestream_(filestream), current_probelm_number_(0) {}

std::optional<core::InputRows> ModifiedPrimalReader::getNextProblem() {

  if (!filestream_.is_open()) {
    std::cout << "ERROR: Unable to open file" << std::endl;
  }
  // initialise strings for storing elements read from file
  std::string temp_string;
  std::string num_variables_string;
  std::string num_inequality_rows_string;

  // get number of variables and number of inequalities we are reading in
  std::getline(filestream_, num_variables_string);
  const int num_variables = atoi(num_variables_string.c_str()) + 1;

  std::getline(filestream_, num_inequality_rows_string);
  const int num_inequality_rows = atoi(num_inequality_rows_string.c_str());

  // initialise vector for using as temporary holding and vector to hold
  // inequality rows
  std::vector<float> matrix_row;
  std::vector<std::vector<float>> inequality_rows;

  // read inequality in rows, add vectors to problem matrix
  for (size_t i = 0; i < num_inequality_rows; ++i) {
    std::getline(filestream_, temp_string);
    matrix_row = convertStringToVector(temp_string);
    inequality_rows.push_back(matrix_row);
    matrix_row.clear();
  }

  // read in equality rows
  std::string num_equality_rows_string;
  std::getline(filestream_, num_equality_rows_string);

  const int num_equality_rows = atoi(num_equality_rows_string.c_str());
  std::vector<std::vector<float>> equality_rows;

  for (size_t i = 0; i < num_equality_rows; ++i) {
    // get and typecast row vector
    std::getline(filestream_, temp_string);
    matrix_row = convertStringToVector(temp_string);
    equality_rows.push_back(matrix_row);
    matrix_row.clear();
  }

  // check we are where we think we are in problem
  std::getline(filestream_, temp_string);
  if (temp_string.find(tilde_) == std::string::npos) {
    std::cout << "Error: Unable to deduce location in problem" << std::endl;
    std::cout << "Last known problem location: " << current_probelm_number_
              << std::endl;
    return std::nullopt;
  }

  // increment filestream problem location
  ++current_probelm_number_;

  // format and return problem
  core::InputRows next_problem;
  next_problem.inequality_rows = inequality_rows;
  next_problem.equality_rows = equality_rows;
  next_problem.num_variables = num_variables;

  return next_problem;
}

std::vector<float>
ModifiedPrimalReader::convertStringToVector(const std::string vector_string) {

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
  std::vector<float> rowvector;
  float temp_float;
  std::string space = " ";

  // if string in vector is not empty or a space, convert to int and add to
  // return vector
  for (size_t i = 0; i < stringvec.size(); ++i) {
    tempstring = stringvec.at(i);
    if (tempstring.compare(space) != 0 && !tempstring.empty()) {
      temp_float = static_cast<float>(atoi(tempstring.c_str()));
      rowvector.push_back(temp_float);
    }
  }
  return rowvector;
}

} // namespace utils
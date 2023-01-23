#include "dual_reader.hpp"
#include <iostream>

namespace utils {
DualReader::DualReader() {}

void DualReader::readFirstProblem(const std::string problems_filepath) {

  // initialise filestream and open file
  std::fstream newfile;
  newfile.open(problems_filepath, std::ios::in);
  // check file is open
  if (!newfile.is_open()) {
    std::cout << "ERROR: Unable to open file" << std::endl;
  }
  // initialise strings for storing elements read from file
  std::string temp_string;
  std::string num_variables_string;
  std::string num_inequality_rows_string;

  // get number of variables and number of inequalities we are reading in
  std::getline(newfile, num_variables_string);
  const int num_variables = atoi(num_variables_string.c_str()) + 1;

  std::getline(newfile, num_inequality_rows_string);
  const int num_inequality_rows = atoi(num_inequality_rows_string.c_str());

  // initialise vector for using as temporary holding for ma`trix row
  std::vector<float> matrix_row;

  // read inequality in rows, add vectors to problem matrix
  for (size_t i = 0; i < num_inequality_rows; ++i) {
    std::getline(newfile, temp_string);
    matrix_row = convertStringToVector(temp_string);
    // check vector size
    if (matrix_row.size() != num_variables) {
      std::cout << "ERROR: length of matrix row is: " << matrix_row.size()
                << " but length of " << num_inequality_rows << " was expected"
                << std::endl;
    }
    problem_matrix_.push_back(matrix_row);
    matrix_row.clear();
  }

  // read in equality rows and add to problem matrix
  // TODO: fix bounds and then add nrgative of row leq also to define equality.
  std::string num_equality_rows_string;
  std::getline(newfile, num_equality_rows_string);

  int num_equality_rows = atoi(num_equality_rows_string.c_str());

  for (size_t i = 0; i < num_equality_rows; ++i) {
    // get and typecast row vector
    std::getline(newfile, temp_string);
    matrix_row = convertStringToVector(temp_string);

    // check vector size
    if (matrix_row.size() != num_variables) {
      std::cout << "ERROR: Expexted row of length " << num_variables
                << " but row has length " << matrix_row.size() << std::endl;
    }
    problem_matrix_.push_back(matrix_row);
    matrix_row.clear();
  }
  newfile.close();

  // make bound vectors
  for (size_t i = 0; i < num_inequality_rows; ++i) {
    upper_bounds_.push_back(core::kIntInfinity);
    lower_bounds_.push_back(0);
  }

  for (size_t i = num_inequality_rows;
       i < num_equality_rows + num_inequality_rows; ++i) {
    upper_bounds_.push_back(0);
    lower_bounds_.push_back(0);
  }
}

void DualReader::readNextProblem(std::fstream &problem_file) {}

std::vector<std::vector<float>> DualReader::getProblemMatrix() {
  return problem_matrix_;
}

std::vector<float> DualReader::getLowerBounds() { return lower_bounds_; }

std::vector<float> DualReader::getUpperBounds() { return upper_bounds_; }

std::vector<float>
DualReader::convertStringToVector(const std::string vector_string) {

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
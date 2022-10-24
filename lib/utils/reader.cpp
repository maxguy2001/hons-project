#include "reader.hpp"

#include <iostream>
#include <fstream>

namespace utils{

Reader::Reader(){}

//this function is probably now useless but still good practise!
void Reader::readFirstProblem(const std::string problems_filepath){

  //initialise filestream and open file
  std::fstream newfile;
  newfile.open(problems_filepath, std::ios::in);
  //check file is open
  if(!newfile.is_open()){
    std::cout << "ERROR: Unable to open file" << std::endl;
  }
  //initialise strings for storing elements read from file
  std::string temp_string;
  std::string num_variables_string;
  std::string num_inequality_rows_string;

  // get number of variables and number of inequalities we are reading in
  std::getline(newfile, num_variables_string);
  const int num_variables = static_cast<int>(atoi(num_variables_string.c_str())) + 1;

  std::getline(newfile, num_inequality_rows_string);
  const int num_inequality_rows = static_cast<int>(atoi(num_inequality_rows_string.c_str()));

  //initialise vector for using as temporary holding for matrix row
  std::vector<int> problem_row;
  int constant_term;
  std::vector<int> problem_matrix_row;

  //read inequality in rows, add vectors to problem matrix
  for(size_t i = 0; i < num_inequality_rows; ++i){
    std::getline(newfile, temp_string);
    problem_row = getProblemRowAsIntVector(temp_string);

    //check vector size
    if(problem_row.size() != num_variables){
      std::cout << "ERROR: length of matrix row is: " << problem_row.size() << " but length of " << num_inequality_rows << " was expected" << std::endl;
    }

    constant_term = problem_row.at(0)*(-1);
    problem_matrix_row = std::vector<int>(problem_row.begin() + 1, problem_row.end());

    problem_matrix_.push_back(problem_matrix_row);
    upper_bounds_.push_back(kMaxInt);
    lower_bounds_.push_back(constant_term);

    problem_row.clear();
    problem_matrix_row.clear();
  }

  //read in equality rows and add to problem matrix
  std::string num_equality_rows_string;
  std::getline(newfile, num_equality_rows_string);

  int num_equality_rows = static_cast<int>(atoi(num_equality_rows_string.c_str()));

  for(size_t i = 0; i < num_equality_rows; ++i){
    std::getline(newfile, temp_string);
    problem_row = getProblemRowAsIntVector(temp_string);

    //check vector size
    if(problem_row.size() != num_variables){
      std::cout << "ERROR: length of matrix row is: " << problem_row.size() << " but length of " << num_inequality_rows << " was expected" << std::endl;
    }

    constant_term = problem_row.at(0)*(-1);
    problem_matrix_row = std::vector<int>(problem_row.begin() + 1, problem_row.end());

    problem_matrix_.push_back(problem_matrix_row);
    upper_bounds_.push_back(constant_term);
    lower_bounds_.push_back(constant_term);

    problem_row.clear();
    problem_matrix_row.clear();
  }
  newfile.close();
}

void Reader::readProblem(const std::string problems_filepath, const int problem_number){
  //clear anything still sored in class members from previous read
  problem_matrix_.clear();

  //initialise filestream and open file
  std::fstream newfile;
  newfile.open(problems_filepath, std::ios::in);
  //check file is open
  if(!newfile.is_open()){
    std::cout << "ERROR: Unable to open file" << std::endl;
  }
  //find location in file we want to look at (nth problem)
  int current_problem_location = 0;
  std::string tempstring;
  std::string tilde = "~";
  while(current_problem_location != problem_number-1){
    std::getline(newfile, tempstring);
    if(tempstring.find(tilde) != std::string::npos){
      current_problem_location += 1;
    }
  }
  
  //initialise strings for storing elements read from file
  std::string temp_string;
  std::string num_variables_string;
  std::string num_inequality_rows_string;

  // get number of variables and number of inequalities we are reading in
  std::getline(newfile, num_variables_string);
  const int num_variables = static_cast<int>(atoi(num_variables_string.c_str())) + 1;

  std::getline(newfile, num_inequality_rows_string);
  const int num_inequality_rows = static_cast<int>(atoi(num_inequality_rows_string.c_str()));

  std::vector<int> problem_row;
  int constant_term;
  std::vector<int> problem_matrix_row;

  //read inequality in rows, add vectors to problem matrix
  for(size_t i = 0; i < num_inequality_rows; ++i){
    std::getline(newfile, temp_string);
    problem_row = getProblemRowAsIntVector(temp_string);

    //check vector size
    if(problem_row.size() != num_variables){
      std::cout << "ERROR: length of matrix row is: " << problem_row.size() << " but length of " << num_inequality_rows << " was expected" << std::endl;
    }

    constant_term = problem_row.at(0)*(-1);
    problem_matrix_row = std::vector<int>(problem_row.begin() + 1, problem_row.end());

    problem_matrix_.push_back(problem_matrix_row);
    upper_bounds_.push_back(kMaxInt);
    lower_bounds_.push_back(constant_term);

    problem_row.clear();
    problem_matrix_row.clear();
  }

  //read in equality rows and add to problem matrix
  std::string num_equality_rows_string;
  std::getline(newfile, num_equality_rows_string);

  int num_equality_rows = static_cast<int>(atoi(num_equality_rows_string.c_str()));

  for(size_t i = 0; i < num_equality_rows; ++i){
    std::getline(newfile, temp_string);
    problem_row = getProblemRowAsIntVector(temp_string);

    //check vector size
    if(problem_row.size() != num_variables){
      std::cout << "ERROR: length of matrix row is: " << problem_row.size() << " but length of " << num_inequality_rows << " was expected" << std::endl;
    }

    constant_term = problem_row.at(0)*(-1);
    problem_matrix_row = std::vector<int>(problem_row.begin() + 1, problem_row.end());

    problem_matrix_.push_back(problem_matrix_row);
    upper_bounds_.push_back(constant_term);
    lower_bounds_.push_back(constant_term);

    problem_row.clear();
    problem_matrix_row.clear();
  }

  newfile.close();
}


std::vector<int> Reader::convertStringToVector(const std::string vector_string){

  std::string tempstring;
  std::vector<std::string> stringvec;

  //convert single string to vector of strings with space character as delimiter
  for(size_t i = 0; i < vector_string.length(); ++i){
    tempstring.push_back(vector_string.at(i));
    if(vector_string.at(i) == ' '){
      stringvec.push_back(tempstring);
      tempstring.clear();
    }
    if(i == vector_string.length()-1){
      stringvec.push_back(tempstring);
    }
  }

  //initialise variables
  std::vector<int> rowvector;
  int temp_int;
  std::string space = " ";

  //if string in vector is not empty or a space, convert to int and add to return vector
  for(size_t i = 0; i < stringvec.size(); ++i){
    tempstring = stringvec.at(i);
    if(tempstring.compare(space) != 0 && !tempstring.empty()){
      temp_int = static_cast<int>(atoi(tempstring.c_str()));
      rowvector.push_back(temp_int);
    }
  }
  return rowvector;
}

std::vector<int> Reader::getProblemRowAsIntVector(const std::string problem_row_string){
  std::string tempstring;
  std::vector<int> row_vector;

  //convert single string to vector of strings with space
  //character as delimiter
  for(size_t i = 0; i < problem_row_string.length(); ++i){
    char position_char = problem_row_string.at(i);
    
    if(position_char != ' '){
      tempstring.push_back(position_char);
    } else {
      row_vector.push_back(static_cast<int>(atoi(tempstring.c_str())));
      tempstring.clear();
    }
  }

  return row_vector;
}

}//namespace utils


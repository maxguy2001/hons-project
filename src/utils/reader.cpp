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
  std::string num_rows_string;
  //skip first line, read second line (number of rows to read 
  //for problem matrix) and convert to integer
  std::getline(newfile, temp_string);
  std::getline(newfile, num_rows_string);
  const int num_rows = atoi(num_rows_string.c_str());

  //read in rows, add vectors to problem matrix
  for(size_t i = 0; i < num_rows; ++i){
    std::getline(newfile, temp_string);
    std::vector<int> matrix_row = convertStringToVector(temp_string);
    problem_matrix_.push_back(matrix_row);
  }

  // get cost vector (2 lines below)
  std::string cost_vector_string;
  std::getline(newfile, cost_vector_string);
  std::getline(newfile, cost_vector_string);
  std::vector<int> cost_vector = convertStringToVector(cost_vector_string);
  for(size_t i = 0; i < cost_vector.size(); ++i){
    cost_vector_.push_back(cost_vector.at(i));
  }
  newfile.close();
}

void Reader::readProblem(const std::string problems_filepath, const int problem_number){
  //clear anything still sored in class members from previous read
  problem_matrix_.clear();
  cost_vector_.clear();

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
  //get number of rows to read for matrix
  std::string numrows_string;
  std::getline(newfile, numrows_string);
  std::getline(newfile, numrows_string);
  const int num_rows = atoi(numrows_string.c_str());

  //get all rows, convert to integer vectors and add to matrix
  std::vector<int> temp_vec;
  for(size_t i = 0; i < num_rows; ++i){
    std::getline(newfile, tempstring);
    temp_vec = convertStringToVector(tempstring);
    problem_matrix_.push_back(temp_vec);
  }

  //get cost vector line from file
  std::string cost_vector_string;
  std::getline(newfile, cost_vector_string);
  std::getline(newfile, cost_vector_string);

  //convert cost vector string to vector of ints and save as class member
  const std::vector<int> cost_vector = convertStringToVector(cost_vector_string);
  for(size_t i = 0; i < cost_vector.size(); ++i){
    cost_vector_.push_back(cost_vector.at(i));
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
      temp_int = atoi(tempstring.c_str());
      rowvector.push_back(temp_int);
    }
  }
  return rowvector;
}

}//namespace utils


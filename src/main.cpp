#include <fstream>
#include <sstream>
#include <iostream>

#include <boost/process.hpp>
namespace bp = boost::process;
#include <string>
using std::string;



void readTasksList(string);


int main(int argc, char* argv[])
{
  std::cout << "Hello World!" << std::endl;

  int return_code = 0;
  string input_file;

  if (argc > 1) input_file = argv[1];
  else input_file = "./input.txt";
  std::cout << "Input file is : " << input_file << std::endl;
  readTasksList(input_file);

  return return_code;
}


void readTasksList(string input_file)
{
  system("clear");
  std::cout << "Initialising machine...\n";
  std::ifstream myFile(input_file);
  string line;
  
  std::cout << "File opened." << std::endl;
  while (std::getline(myFile, line))
  {
    std::cout << "Managing line : " << line << std::endl;
    int result = bp::system("./" + line);
  }

}

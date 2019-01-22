#include <fstream>
#include <sstream>
#include <iostream>
#include <stdio.h>
#include <signal.h>

#include <alchemy/task.h>

#include <string>
using std::string;

RT_TASK mcAgent;

void readTasksList(string);
void mcAgentMain(void *arg);


int main(int argc, char* argv[])
{
  int return_code = 0;

  /* To create a task :
   * Arguments : &task,
   *             name,
   *             stack size (def = 0)
   *             priority
   *             mode (FPU, Start suspended, ...)
   */
  rt_task_create(&mcAgent, "Hello", 0, 50, 0);

  /* To start a task
   * Arguments : &task,
   *             task main
   *             function Arguments
   */

   rt_task_start(&mcAgent, &mcAgentMain, 0);

/*
  // get input file, either indicated by user as argument or default location
  string input_file;
  if (argc > 1) input_file = argv[1];
  else input_file = "./input.txt";
  std::cout << "Input file is : " << input_file << std::endl;
  readTasksList(input_file);
*/
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
    string str = "./" + line;
    char *cmd = &str[0u];
    int result = system(cmd);
  }
}


void mcAgentMain(void *arg)
{
  RT_TASK_INFO curtaskinfo;

  std::cout << "Hello World !" << std::endl;
  rt_task_inquire(NULL,&curtaskinfo);

  std::cout << "I am task : " << curtaskinfo.name << std::endl;
}

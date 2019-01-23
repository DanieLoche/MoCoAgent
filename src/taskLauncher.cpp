#include "tools.h"

#include "mcAgent.h"
#include "macroTask.h"


void readTasksList(string);
void RunmcAgentMain(void *arg);
void RunTask(void* arg);

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
  RT_TASK mcAgent;
  int rep = rt_task_create(&mcAgent, "Hello", 0, 50, 0);

  /* To start a task
   * Arguments : &task,
   *             task main
   *             function Arguments
   */
   rtTaskInfos rtTI = { mcAgent, "Hello", "./bin/testApp"};
   rt_task_start(&mcAgent, RunTask, &rtTI);

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

void RunmcAgentMain(void *arg)
{
  MCAgent* mcAgent = new MCAgent();
}



void RunTask(void* arg)
{
  rtTaskInfos rtTI = *(rtTaskInfos*) arg;

  std::cout << "name : " << rtTI.name << " path: " << rtTI.path << std::endl;
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

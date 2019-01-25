#include "tools.h"

#include "mcAgent.h"
#include "macroTask.h"
#include "taskLauncher.h"

void RunmcAgentMain(void *arg)
{
  MCAgent* mcAgent = new MCAgent();
}


void TaskMain(void* arg)
{
  /*
  std::cout << "Running !!!" << std::endl;
  rtTaskInfosStruct* rtTI = (rtTaskInfosStruct*) arg;
  //printTaskInfo (rtTI);

  RT_TASK_INFO curtaskinfo;
  rt_task_inquire(NULL, &curtaskinfo);
  std::cout << "I am task : " << curtaskinfo.name << " of priority " << curtaskinfo.prio << std::endl;
  */
  MacroTask macroRT;

  macroRT.executeRun();

}

int main(int argc, char* argv[])
{
  int return_code = 0;

  // get input file, either indicated by user as argument or default location
  string input_file;
  if (argc > 1) input_file = argv[1];
  else input_file = "./input.txt";

  TaskLauncher tln(input_file);
  //tln.tasksInfos = readTasksList(input_file);

  tln.printTasksInfos();
  tln.runTasks();
  /* To create a task :
   * Arguments : &task,
   *             name,
   *             stack size (def = 0)
   *             priority
   *             mode (FPU, Start suspended, ...)
   */
//  RT_TASK mcAgent;
//  int rep = rt_task_create(&mcAgent, "Hello", 0, 50, 0);

  /* To start a task
   * Arguments : &task,
   *             task main
   *             function Arguments
   */
//   rt_task_start(&mcAgent, RunTask, &tasksInfos);


  return return_code;
}

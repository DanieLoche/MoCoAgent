#include "tools.h"

#include "mcAgent.h"
#include "macroTask.h"
#include "taskLauncher.h"

void printTaskInfo(rtTaskInfosStruct* task)
{
  cout << "Name: " << task->name
      << "| path: " << task->path
      << "| is RT ? " << task->isHardRealTime
      << "| Period: " << task->periodicity
      << "| Deadline: " << task->deadline
      << "| affinity: " << task->affinity << endl;
}

void RunmcAgentMain(void *arg)
{
  cout << " I am working" << endl;
  MCAgent* mcAgent = new MCAgent();
  mcAgent->mcAgentMain(0);
}


void TaskMain(void* arg)
{
  rtTaskInfosStruct* rtTI = (rtTaskInfosStruct*) arg;

  RT_TASK_INFO curtaskinfo;
  rt_task_inquire(NULL, &curtaskinfo);
  cout << "I am task : " << curtaskinfo.name << " of priority " << curtaskinfo.prio << endl;

  MacroTask macroRT;
  macroRT.properties = *rtTI;
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


  return return_code;
}

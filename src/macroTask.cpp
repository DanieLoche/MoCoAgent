#include "macroTask.h"

extern void print_affinity(pid_t _pid);
extern void printTaskInfo(rtTaskInfosStruct* task);

MacroTask::MacroTask()
{

}

int MacroTask::before()
{
  print_affinity(0);
  return 0;
}

void MacroTask::proceed()
{
  char* cmd;
  cout << "path :" << properties.path << "." << endl;
  if (properties.path != "/null/")
  {
    cmd = &properties.path[0u];
    system(cmd);
  }
  else cout << "Oups, no valid path found !" << endl;
}

int MacroTask::after()
{
  return 0;
}

void MacroTask::executeRun()
{
  cout << "Running..." << endl;
  before(); // Check if execution allowed

  proceed();  // execute task

  if (properties.isHardRealTime)
    after();  // Inform of execution time for the mcAgent
}

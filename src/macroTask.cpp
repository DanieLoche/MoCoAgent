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

void MacroTask::proceed(RT_SEM* mysync)
{
  char* cmd;
  cout << "path :" << properties.path << "." << endl;
  rt_sem_p(mysync,TM_INFINITE);
  if (properties.path != "/null/")
  {
    cmd = &properties.path[0u];
    system(cmd);
  }
  else cout << properties.name <<"Oups, no valid path found !" << endl;
}

int MacroTask::after()
{
  return 0;
}

void MacroTask::executeRun(RT_SEM* mysync)
{
  cout << "Running..." << endl;
  before(); // Check if execution allowed

  proceed(mysync);  // execute task

  if (properties.isHardRealTime)
    after();  // Inform of execution time for the mcAgent
}

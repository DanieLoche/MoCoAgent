#include "macroTask.h"

MacroTask::MacroTask()
{

}

int MacroTask::before()
{
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

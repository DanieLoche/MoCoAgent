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
  if (properties.path != "")
    char *cmd = &properties.path[0u];
  system(cmd);
}

int MacroTask::after()
{
  return 0;
}

void MacroTask::executeRun()
{
  before(); // Check if execution allowed

  proceed();  // execute task

  if (properties.isHardRealTime)
    after();  // Inform of execution time for the mcAgent
}

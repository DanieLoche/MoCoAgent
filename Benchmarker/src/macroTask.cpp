#include "macroTask.h"
#include <signal.h>
#include <time.h>
#include <alchemy/timer.h>

extern void print_affinity(pid_t _pid);
extern void printTaskInfo(rtTaskInfosStruct* task);

MacroTask::MacroTask()
{

}

int MacroTask::before()
{
  this->set_time(0);
  print_affinity(0);
  return 0;
}

void MacroTask::proceed()
{
  char* cmd;
  cout << "path :" << properties->path << "." << endl; cout.flush();
  if (properties->path != "/null/")
  {
    cmd = &properties->path[0u];
    system(cmd);
  }
  else
  {
    cout << "Oups, no valid path found !" << endl;
    cout.flush();
  }
}

int MacroTask::after()
{

  this->set_time(1);
  this->compute_time();
  return 0;
}

void MacroTask::executeRun()
{
  cout << "Running..." << endl; cout.flush();
  before(); // Check if execution allowed

  proceed();  // execute task

  //if (properties.isHardRealTime)
  after();  // Inform of execution time for the mcAgent

}

void MacroTask::set_time(int input)
{
  cout.precision(17);
  switch (input) {
    case 0 :
      start_time_ms = rt_timer_read()/ 1.0e6;
      break;
    case 1 :
      end_time_ms = rt_timer_read()/ 1.0e6;
      break;
  }
}

void MacroTask::compute_time()
{
  cout.precision(17);
  execution_time_ms = (end_time_ms - start_time_ms );
   // Convert seconds to miliseconds
}

double MacroTask::get_execution_time()
{
  return execution_time_ms;
}

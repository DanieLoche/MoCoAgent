#include "mcAgent.h"

MCAgent::MCAgent()
{

  taskList = new RT_TASK[taskListSize];
}

void MCAgent::mcAgentMain(void *arg)
{
  RT_TASK_INFO curtaskinfo;

  rt_task_inquire(NULL, &curtaskinfo);

  cout << "I am task : " << curtaskinfo.name << "PID : " << curtaskinfo.pid << ", of priority " << curtaskinfo.prio << endl;
  cout << curtaskinfo.pid << " : " << "executed in primary for " << curtaskinfo.stat.xtime << " ns" << endl;
}

#include "mcAgent.h"

MCAgent::MCAgent()
{

  taskList = new RT_TASK[taskListSize];
  mcAgentMain(0);
}

void MCAgent::mcAgentMain(void *arg)
{
  RT_TASK_INFO curtaskinfo;

  cout << "Hello World !" << endl;
  rt_task_inquire(NULL, &curtaskinfo);

  cout << "I am task : " << curtaskinfo.name << endl;
}

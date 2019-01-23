#include "mcAgent.h"

MCAgent::MCAgent()
{
  
  taskList = new RT_TASK[taskListSize];
  mcAgentMain(0);
}

void MCAgent::mcAgentMain(void *arg)
{
  RT_TASK_INFO curtaskinfo;

  std::cout << "Hello World !" << std::endl;
  rt_task_inquire(NULL, &curtaskinfo);

  std::cout << "I am task : " << curtaskinfo.name << std::endl;
}

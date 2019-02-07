#include "mcAgent.h"

#define MODE_OVERLOADED 1
#define MODE_NOMINAL    0

extern void print_affinity(pid_t _pid);
extern void printTaskInfo(rtTaskInfosStruct* task);

MCAgent::MCAgent(void *arg)
{
  RT_TASK_INFO curtaskinfo;
  rt_task_inquire(NULL, &curtaskinfo);
  #if VERBOSE_INFO
  cout << "I am task : " << curtaskinfo.name << " (PID : " << curtaskinfo.pid << "), of priority " << curtaskinfo.prio << endl;
  #endif
  //print_affinity(0);

  std::vector<rtTaskInfosStruct>* rtTI = (std::vector<rtTaskInfosStruct>*) arg;
  TasksInformations = rtTI;

  displayInformations();

    while(false)
    {
      if (runtimeMode == MODE_OVERLOADED)
      {

      } else if(runtimeMode == MODE_NOMINAL)
      {

      }
      checkTasks();
    }
}

int MCAgent::checkTasks()
{
  for (auto _taskChain = allTaskChain.begin(); _taskChain != allTaskChain.end(); ++_taskChain)
  {
    if (_taskChain->checkTaskRT() )
    {
      setMode(MODE_OVERLOADED);
    }
  }
  return 0;
}

void MCAgent::setMode(int mode)
{
  runtimeMode = mode;
  // Parcourir toutes les tâches best effort, et soir les suspendre, soit les reprendre.
}

void MCAgent::displayInformations()
{
  #if VERBOSE_INFO
  for (auto &taskInfo : *TasksInformations)
  {
      cout << "Name: " << taskInfo.name
          << "| path: " << taskInfo.path
          << "| is RT ? " << taskInfo.isHardRealTime
          << "| Period: " << taskInfo.periodicity
          << "| Deadline: " << taskInfo.deadline
          << "| affinity: " << taskInfo.affinity << endl;
  }
  #endif
}

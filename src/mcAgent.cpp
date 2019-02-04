#include "mcAgent.h"


extern void print_affinity(pid_t _pid);
extern void printTaskInfo(rtTaskInfosStruct* task);

MCAgent::MCAgent(void *arg)
{
  RT_TASK_INFO curtaskinfo;

  rt_task_inquire(NULL, &curtaskinfo);

  cout << "I am task : " << curtaskinfo.name << " (PID : " << curtaskinfo.pid << "), of priority " << curtaskinfo.prio << endl;
  //print_affinity(0);

  std::vector<rtTaskInfosStruct>* rtTI = (std::vector<rtTaskInfosStruct>*) arg;
  setTasksInfos(rtTI);

  displayInformations();

    while(false)
    {
      mcAgent.checkTasks();
    }
}

void MCAgent::setTasksInfos(std::vector<rtTaskInfosStruct>* _tasksInfos)
{
  TasksInformations = _tasksInfos;

}

int MCAgent::checkTasks()
{
  return 0;
}

void MCAgent::displayInformations()
{
  for (auto &taskInfo : *TasksInformations)
  {
      cout << "Name: " << taskInfo.name
          << "| path: " << taskInfo.path
          << "| is RT ? " << taskInfo.isHardRealTime
          << "| Period: " << taskInfo.periodicity
          << "| Deadline: " << taskInfo.deadline
          << "| affinity: " << taskInfo.affinity << endl;
  }
}

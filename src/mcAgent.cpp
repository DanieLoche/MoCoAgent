#include "mcAgent.h"


extern void print_affinity(pid_t _pid);
extern void printTaskInfo(rtTaskInfosStruct* task);

MCAgent::MCAgent(void *arg)
{
  RT_TASK_INFO curtaskinfo;

  rt_task_inquire(NULL, &curtaskinfo);

  cout << "I am task : " << curtaskinfo.name << " (PID : " << curtaskinfo.pid << "), of priority " << curtaskinfo.prio << endl;
  print_affinity(0);
  cout << curtaskinfo.pid << " : " << "executed in primary for " << curtaskinfo.stat.xtime << " ns" << endl;
  2*7/9;
}

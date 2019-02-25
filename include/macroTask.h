#include "tools.h"
#include <algorithm>
#include <mutex>

class MacroTask
{
  private :
    /* rtTaskInfosStruct :
          RT_TASK*  task;
          char      name[64];
          string    path;
          string    task_args;
          bool      isHardRealTime;
          int       periodicity;
          int       deadline;
          int       affinity;
    */
    RTIME starttime, runtime,endtime,time,Somme;
    int cpt;
    string task_path;
    std::mutex mutex;
    int before();
    void proceed();
    int after();

  public :
    rtTaskInfosStruct* properties;

    MacroTask();
    void executeRun(RT_SEM* mysync ,RT_BUFFER* bf);

};

extern void printTaskInfo(rtTaskInfosStruct* task);

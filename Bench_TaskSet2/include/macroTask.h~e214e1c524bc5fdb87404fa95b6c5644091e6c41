#include "tools.h"
#include <algorithm>

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
    int before();
    void proceed();
    int after();

  public :
    rtTaskInfosStruct* properties;
  //  struct rusage usage;

    MacroTask();
    void executeRun();
};

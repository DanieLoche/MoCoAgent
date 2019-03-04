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
    double execution_time_ms;
    RTIME start_time_ms;
    RTIME end_time_ms;
  public :
    rtTaskInfosStruct* properties;
  //  struct rusage usage;
    double get_execution_time();
    void compute_time();
    void set_time(int input);

    MacroTask();
    void executeRun();
};

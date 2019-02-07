#include "tools.h"

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
    int before();
    void proceed();
    int after();
    RTIME start_time_ms;
    RTIME end_time_ms;
    RTIME execution_time_ms;
    pid_t main_pid;

  public :
    rtTaskInfosStruct properties;

    MacroTask();
    void executeRun();
    double get_execution_time();
    void set_time(int input);
    void compute_time();
    void set_main_pid(pid_t pid);
};

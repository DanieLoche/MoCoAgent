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

  public :
    rtTaskInfosStruct properties;

    MacroTask();
    void executeRun();

};

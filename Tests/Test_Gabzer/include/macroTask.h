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
    int before();
    void proceed(RT_SEM* mysync);
    int after();

  public :
    rtTaskInfosStruct* properties;

    MacroTask();
    void executeRun(RT_SEM* mysync);

};

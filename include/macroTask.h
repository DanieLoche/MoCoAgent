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
    rtTaskInfosStruct properties;

    int before();
    void proceed();
    int after();

  public :
    MacroTask();
    void executeRun();

};


class MCAgent
{
  private :
    int taskListSize;
    RT_TASK *taskList;
  public :
    MCAgent();
    void mcAgentMain(void *arg);

};

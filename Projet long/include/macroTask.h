#include "tools.h"
#include <algorithm>
#include <mutex>

class MacroTask
{
  protected :
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
    RT_BUFFER bf;
    RT_EVENT	event;
    int before_besteff();
    void proceed();
    int after_besteff();
    int before();
    int after();

  public :
    rtTaskInfosStruct* properties;

    MacroTask();
    void executeRun(RT_SEM* mysync);
    void executeRun_besteffort(RT_SEM* mysync);

};

/*
class MacroTaskBestEffort : public MacroTask
{
  protected :
    void executeRun(RT_SEM* mysync);

  public :
    int before();
    int after();
};
*/
extern void printTaskInfo(rtTaskInfosStruct* task);

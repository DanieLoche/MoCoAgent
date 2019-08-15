#include "tools.h"
#include "dataLogger.h"
#include <algorithm>

class MacroTask
{
  protected :
    /* struct rtTaskInfosStruct
    {
         char   name[32];
         char   path_task[128];
         string arguments;
         int    isHardRealTime;     // task chain ID or best effort if null
         int    id;
         int    affinity;
         int    precedency;
         int    priority;
         RTIME  wcet;
         RTIME  periodicity;
         RT_TASK* task;
    } ;
    */
    rtTaskInfosStruct* properties;
    TaskDataLogger* dataLogs;

    RT_BUFFER bf;
    RT_EVENT	event;
//    RT_MUTEX mutex;
    bool MoCoIsAlive;
    int priority;
    monitoringMsg msg ;
    string chain;
    char **argv;
    char* stdIn;
    char* stdOut;

    inline int before_besteff();
    inline int before();
    inline void proceed();
    inline int after();
    inline int after_besteff();

  public :
    MacroTask(taskRTInfo*, bool);
    void executeRun();
    void executeRun_besteffort();

};

extern void print_affinity(pid_t _pid);
extern void printTaskInfo(rtTaskInfosStruct* task);

extern void print_affinity(pid_t _pid);
extern void printTaskInfo(rtTaskInfosStruct* task);

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

//extern void printTaskInfo(rtTaskInfosStruct* task);

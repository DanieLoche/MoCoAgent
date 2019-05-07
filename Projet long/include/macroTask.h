#include "tools.h"
#include "dataLogger.h"
#include <algorithm>

class MacroTask
{
  protected :
    /* struct rtTaskInfosStruct
    {
        char name[64];
        char path_task[128];
        int  isHardRealTime;
        RTIME  periodicity;
        RTIME  deadline;
        int affinity;
        int ID;
        RT_TASK* task;
    } ;
    */
    rtTaskInfosStruct* properties;
    TaskDataLogger* dataLogs;

    RT_BUFFER bf;
    RT_EVENT	event;
    bool MoCoIsAlive;

    monitoringMsg msg ;
    char* cmd;

    int before_besteff();
    void proceed();
    int after_besteff();
    int before();
    int after();

  public :
    MacroTask(taskRTInfo*);
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

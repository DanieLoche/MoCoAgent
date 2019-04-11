#include "tools.h"
#include "dataLogger.h"

class TaskLauncher
{
  private :
    int cptNumberTasks ;
    systemRTInfo taskSetInfos;
    std::vector<DataLogger*> tasksLogsList;
  public :
    TaskLauncher();

    void runTasks();
    void runAgent();
    void readChainsList(string);
    int  readTasksList();

    void saveData();
    void printTasksInfos (/* std::vector<rtTaskInfosStruct> _myTasksInfos*/);
    void print_affinity(pid_t _pid);

};

extern void RunmcAgentMain(void *arg);
extern void TaskMain(void* arg);
extern void print_affinity(pid_t _pid);
extern void printTaskInfo(rtTaskInfosStruct* task);
extern void set_affinity (RT_TASK* task, int _aff);

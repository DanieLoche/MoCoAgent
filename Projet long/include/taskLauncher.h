#include "tools.h"
#include "dataLogger.h"

class TaskLauncher
{
   private :
      systemRTInfo taskSetInfos;
      std::vector<DataLogger*> tasksLogsList;
      RT_TASK mcAgent;
//      RT_MUTEX mutexes[8];
      bool enableAgent;
      bool triggerSave;

      void rt_task_affinity (RT_TASK* task, int _aff, int mode);

   public :
      int schedPolicy;
      TaskLauncher(string outputFileName);
      int readChainsList(string);
      int  readTasksList(int cpuPercent);
      int createTasks();
//      int createMutexes(int nprocs);
      int runTasks();
      int runAgent();
      void stopTasks(bool);
      void saveData(string);
      void printTasksInfos (/* std::vector<rtTaskInfosStruct> _myTasksInfos*/);

};

extern void RunmcAgentMain(void *arg);
extern void TaskMain(void* arg);
extern void print_affinity(pid_t _pid);
extern void printTaskInfo(rtTaskInfosStruct* task);

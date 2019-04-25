#include "tools.h"
#include "dataLogger.h"

class TaskLauncher
{
   private :
      int cptNumberTasks ;
      systemRTInfo taskSetInfos;
      std::vector<DataLogger*> tasksLogsList;
      RT_TASK mcAgent;
      bool enableAgent;

      void rt_task_affinity (RT_TASK* task, int _aff, int mode);

   public :
      int schedPolicy;
      TaskLauncher();
      void readChainsList(string);
      int  readTasksList();
      void createTasks();
      void runTasks();
      void runAgent();
      void stopTasks(bool);
      void saveData(string);
      void printTasksInfos (/* std::vector<rtTaskInfosStruct> _myTasksInfos*/);

};

extern void RunmcAgentMain(void *arg);
extern void TaskMain(void* arg);
extern void print_affinity(pid_t _pid);
extern void printTaskInfo(rtTaskInfosStruct* task);

#ifndef TASKLAUNCHER_H
#define TASKLAUNCHER_H

#include "tools.h"
#include "dataLogger.h"
#include "macroTask.h"

class TaskLauncher
{
   private :
      string outputFileName;
      std::vector<end2endDeadlineStruct> e2eDD;
      std::vector<rtTaskInfosStruct> tasksSet;
      std::vector<RT_TASK*> tasks;
      rtTaskInfosStruct currentTaskDescriptor;
      int enableAgent;
      int schedPolicy;

      MCAgent* moCoAgent;
      RT_SEM syncSem;

   public :

      bool triggerSave;
      TaskLauncher(string outputFileName, int schedPolicy);
      int readChainsList(string);
      int readTasksList (int cpuPercent);
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

#endif

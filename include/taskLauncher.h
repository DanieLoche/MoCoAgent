#ifndef TASKLAUNCHER_H
#define TASKLAUNCHER_H

#include "tools.h"
#include "macroTask.h"
#include "dataLogger.h"
#include "sched.h"
#include "edf.h"

#define ALARM_NAME   "endOfExpe_Alarm"
#define SEM_NAME     "Start_Sem"
#define MCA_PERIOD   2 // ms

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
      RT_ALARM _endAlarm;
      RT_SEM _syncSem;

   public :

      bool triggerSave;
      TaskLauncher(string outputFileName, int schedPolicy);
      int readChainsList(string);
      int readTasksList (int cpuPercent);
//      int createMutexes(int nprocs);
      int runTasks(long expeDuration);
      int runAgent(long expeDuration);
      void stopTasks(bool);
      void saveData(string);
      void printTasksInfos (/* std::vector<rtTaskInfosStruct> _myTasksInfos*/);

};

extern void RunmcAgentMain(void *arg);
extern void TaskMain(void* arg);
extern void print_affinity(pid_t _pid);
extern void printTaskInfo(rtTaskInfosStruct* task);

#endif

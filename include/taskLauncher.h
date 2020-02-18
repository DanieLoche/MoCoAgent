#ifndef TASKLAUNCHER_H
#define TASKLAUNCHER_H

#include "tools.h"
#include "macroTask.h"
#include "dataLogger.h"
//#include "sched.h"

#define ALARM_NAME   "endOfExpe_Alarm"
#define SEM_NAME     "Start_Sem"
#define MCA_PERIOD   2 // ms

class TaskLauncher
{
   private :
      static RT_SEM _syncSem;
      static string outputFileName;
      static int nameMaxSize;
      std::vector<end2endDeadlineStruct> e2eDD;
      static std::vector<rtTaskInfosStruct> tasksSet;
      static std::vector<RT_TASK*> tasks;
      rtTaskInfosStruct currentTaskDescriptor;
      int enableAgent;
      int schedPolicy;

      TaskProcess* currentProcess;
      RT_ALARM _endAlarm;

   public :
      //bool triggerSave;
      TaskLauncher(string outputFileName, int schedPolicy);
      ~TaskLauncher(){};

      int readChainsList(string);
      int readTasksList (int cpuPercent);
      //  int createMutexes(int nprocs);
      int runTasks(long expeDuration);
      int runAgent(long expeDuration);
      void stopTasks(bool);
      static void finishProcess(void* _arg /*MCAgent* task*/);
      //void saveData(string);
      void printTasksInfos (/* std::vector<rtTaskInfosStruct> _myTasksInfos*/);

};

//extern void RunmcAgentMain(void *arg);
//extern void TaskMain(void* arg);
extern void print_affinity(pid_t _pid);
extern void printTaskInfo(rtTaskInfosStruct* task);

#endif

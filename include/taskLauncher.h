#ifndef TASKLAUNCHER_H
#define TASKLAUNCHER_H

#include "tools.h"
#include "macroTask.h"
#include "dataLogger.h"
//#include "sched.h"

#define ALARM_NAME   "endOfExpe_Alarm"
#define SEM_NAME     "Start_Sem"

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
      TaskLauncher(int agentMode, string outputFileName, int schedPolicy);
      ~TaskLauncher(){};

      int readChainsList(string);
      int readTasksList (int cpuPercent);
      int runTasks(long expeDuration);
      int runAgent(long expeDuration);
      static void finishMoCoAgent(void* _arg /*MCAgent* task*/);
      static void finishTask(void* _arg /*MCAgent* task*/);
      //void stopTasks(bool);
      //void saveData(string);
      void printTaskSetInfos (/* std::vector<rtTaskInfosStruct> _myTasksInfos*/);
      void printChainSetInfos (/* std::vector<rtTaskInfosStruct> _myTasksInfos*/);

};


#endif

#ifndef TASKS_H
#define TASKS_H

#include <fcntl.h>      // gestion systèmes de fichiers ( dup() )

#include "tools.h"
#include "dataLogger.h"
//#include "taskLauncher.h"

#define  MODE_OVERLOADED          1
#define  MODE_NOMINAL            -1
#define  MODE_DISABLE             0

#define  CHANGE_MODE_EVENT_NAME  "modeChangeEvent"
#define  MESSAGE_TOPIC_NAME      "monitoringTopic"
#define  LOG_MUTEX_NAME          "logToFileMutex"

#define  USE_MUTEX               0

#define   MCA_PERIOD             2 // ms
const RTIME t_RT     =     000*1.0e6;  // time to trigger the Control Agent
const RTIME Wmax     =     _mSEC(MCA_PERIOD);    // next slice max time

class taskMonitoringStruct
{
   private :
      #if USE_MUTEX
      RT_MUTEX mtx_taskStatus;
      #endif
      bool isExecuted; // Run-time - computed

   public :
      int precedencyID;
      RT_TASK xenoTask;
      int id;
      //RTIME endTime;     // Run-time - received
      RTIME deadline;    // Static
      RTIME rwcet;       // Static
      //bool operator <(const taskMonitoringStruct& tms) const {return (id < tms.id);}

      taskMonitoringStruct(rtTaskInfosStruct rtTaskInfos);
      void setState(bool state);
      bool getState();
};

class taskChain
{
   public :
      ChainDataLogger* logger;
      char name[32];
      int chainID;                // static
      RTIME end2endDeadline; // static
      RTIME startTime;       // Runtime - deduced
      RTIME currentEndTime;  // Runtime - deduced
      RTIME remWCET;         // Runtime - computed
      bool isAtRisk;         // Runtime - deduced
      std::vector<taskMonitoringStruct> taskList;

      taskChain(end2endDeadlineStruct _tcDeadline, string outfile);

      bool checkPrecedency(int taskID);
      bool checkTaskE2E();
      bool checkIfEnded();
      void displayTasks();
      void resetChain();
   private:
      RTIME getExecutionTime();
      RTIME getRemWCET();
};

class TaskProcess
{
   protected:
      RT_EVENT	_event;
      RT_EVENT_INFO _eventInfos;
      monitoringMsg msg ;
      char _stdIn[35];
      char _stdOut[50];
      std::vector<char*> _argv;

      void setAffinity (int _aff, int mode);
      void setRTtask(rtPStruct, char*);
      void parseParameters(string _arguments);
      void setIO( );

   public:
      static bool MoCoIsAlive;
      //RT_MUTEX _bufMtx;
      RT_TASK _task;
      RT_BUFFER _buff;

      TaskProcess(rtTaskInfosStruct _taskInfo);
      virtual void executeRun() = 0;
      virtual void executeRun_besteffort() = 0;
      void saveData(int nameMaxSize);
};

class MacroTask : public TaskProcess
{
   protected :
      int ret;
      TaskDataLogger* dataLogs;
      int (*proceed_function)(int Argc, char *argv[]);

      //void setRTtask(rtPStruct _rtInfos, char*);
      void findFunction(char* _func);
      inline int before();
      inline int proceed();
      inline int after();
      inline int before_besteff();
      inline int after_besteff();

   public :
      rtTaskInfosStruct prop;

      MacroTask(rtTaskInfosStruct taskInfos, bool MoCoMode, string name);
      void saveData(int maxNameSize, RT_TASK_INFO* cti = NULL);
      void executeRun();
      void executeRun_besteffort();

};

class MCAgent : public TaskProcess
{
   protected :
      RT_TASK msgReceiverTask;
      //bool enable;
      short runtimeMode;    // NOMINAL or OVERLOADED
      ulong overruns;
      std::vector<taskChain> allTaskChain;
      std::vector<RT_TASK> bestEffortTasks;

      void initCommunications();
      void setAllDeadlines(std::vector<end2endDeadlineStruct> _tcDeadlineStructs);
      void setAllTasks(std::vector<rtTaskInfosStruct> _TasksInfos);
      void setMode(int mode);
      //int checkTaskChains();
      void displaySystemInfo(std::vector<end2endDeadlineStruct> e2eDD,
                             std::vector<rtTaskInfosStruct> tasksSet);
      void displayChains();

   public :
      MCAgent(rtTaskInfosStruct _taskInfo,
      std::vector<end2endDeadlineStruct> e2eDD,
      std::vector<rtTaskInfosStruct> tasksSet);
      void updateTaskInfo(monitoringMsg msg);
      void saveData();
      //static void finishProcess(void* _mcAgent /*MCAgent* task*/);
      void executeRun();
      void executeRun_besteffort();
};

   int do_load           (int argc, char* argv[]);
extern "C" {
   int basicmath_small   (int argc, char *argv[]);
   int basicmath_large   (int argc, char *argv[]);
   int bitcount_func     (int argc, char *argv[]);
   int qsort_small       (int argc, char *argv[]);
   int qsort_large       (int argc, char *argv[]);
   int susan             (int argc, char *argv[]);
   int djpeg_func        (int argc, char *argv[]);
   int cjpeg_func        (int argc, char *argv[]);
   int typeset_func      (int argc, char *argv[]);
   int dijkstra_small    (int argc, char *argv[]);
   int dijkstra_large    (int argc, char *argv[]);
   int patricia          (int argc, char *argv[]);
   int stringsearch_small(int argc, char *argv[]);
   int stringsearch_large(int argc, char *argv[]);
   int blowfish          (int argc, char *argv[]);
   int rijndael          (int argc, char *argv[]);
   int sha               (int argc, char *argv[]);
   int rawdaudio         (int argc, char *argv[]);
   int rawcaudio         (int argc, char *argv[]);
   int crc               (int argc, char *argv[]);
   int fft               (int argc, char *argv[]);
   int gsm_func          (int argc, char *argv[]);
}

#endif

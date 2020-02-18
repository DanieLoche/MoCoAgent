#ifndef TASKS_H
#define TASKS_H

#include <fcntl.h>      // gestion systèmes de fichiers ( dup() )

#include "tools.h"
#include "dataLogger.h"
//#include "taskLauncher.h"

#define   MODE_OVERLOADED     1
#define   MODE_NOMINAL        -1
#define   MODE_DISABLE        0

#define CHANGE_MODE_EVENT_NAME   "/modeChangeTopic"
#define MESSAGE_TOPIC_NAME       "/monitoringTopic"

const RTIME t_RT = 000*1.0e6;  // time to trigger the Control Agent
const RTIME Wmax = 000*1.0e6;    // next slice max time

class taskMonitoringStruct
{
   private :
      RT_MUTEX mtx_taskStatus;
      bool isExecuted; // Run-time - computed

   public :
      int precedencyID;
      RT_TASK* xenoTask;
      int id;
      //RTIME endTime;     // Run-time - received
      RTIME deadline;    // Static
      RTIME rwcet;       // Static
      //bool operator <(const taskMonitoringStruct& tms) const {return (id < tms.id);}

      taskMonitoringStruct(rtTaskInfosStruct* rtTaskInfos);
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

      taskChain(end2endDeadlineStruct _tcDeadline);

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
      std::vector<char*> _argv;
      char stdIn[35];
      char stdOut[35];
      static bool MoCoIsAlive;

      void setAffinity (int _aff, int mode);
      void setRTtask(rtPStruct, char*);
      void parseParameters(string _arguments);
      void setIO(char _stdIn[35], char _stdOut[35]);

   public:
      RT_TASK* _task;
      RT_BUFFER _buff;

      TaskProcess(rtTaskInfosStruct _taskInfo, bool MoCo);
      virtual void executeRun() = 0;
      virtual void executeRun_besteffort() = 0;
};

class MacroTask : public TaskProcess
{
   protected :
      rtTaskInfosStruct prop;
      int (*proceed_function)(int Argc, char *argv[]);

      //void setRTtask(rtPStruct _rtInfos, char*);
      void findFunction(char* _func);
      inline int before();
      inline void proceed();
      inline int after();
      inline int before_besteff();
      inline int after_besteff();

   public :
      TaskDataLogger* dataLogs;

      MacroTask(rtTaskInfosStruct, bool);
      static void finishProcess(void* _task /*TaskProcess* task*/);
      void executeRun();
      void executeRun_besteffort();

};

class MCAgent : public TaskProcess
{
   protected :
      bool enable;
      short runtimeMode;    // NOMINAL or OVERLOADED
      ulong overruns;
      std::vector<taskChain*> allTaskChain;
      std::vector<RT_TASK*> bestEffortTasks;

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

extern void print_affinity(pid_t _pid);
extern void printTaskInfo(rtTaskInfosStruct* task);
//extern std::streambuf *cinbuf, *coutbuf;


//extern void printTaskInfo(rtTaskInfosStruct* task);

#endif

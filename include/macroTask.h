#ifndef TASKS_H
#define TASKS_H

#include <fcntl.h>      // gestion systèmes de fichiers ( dup() )

#include "tools.h"
#include "dataLogger.h"
//#include "taskLauncher.h"

// 1 bit-set flags, non-null !!
#define  MODE_OVERLOADED         (0*1|0*2|1*4)
#define  MODE_NOMINAL            (0*1|1*2)
#define  MODE_DISABLE            (1*1)

#define  CHANGE_MODE_EVENT_NAME  "modeChangeEvent"
#define  MESSAGE_TOPIC_NAME      "monitoringTopic"
#define  LOG_MUTEX_NAME          "logToFileMutex"

#define  USE_MUTEX               0

#define   MCA_PERIOD       _mSEC(5) // ms
const RTIME t_RT     =     _uSEC(500);  // time to trigger the Control Agent
const RTIME Wmax     =     MCA_PERIOD;    // next slice max time

#define XENO_INIT(_name) do_xeno_init(_name)
int do_xeno_init(char* name);

struct ExecTimes
{
   RTIME start;
   RTIME end;
};

#define WITH_BOOL   0
struct monitoringMsg
{
   //RT_TASK* task;
   uint ID;
   RTIME time;   // Run-time - task start received
   RTIME endTime; // task duration
   #if WITH_BOOL
   bool isExecuted;    // Run-time - computed
   #endif
};

class taskMonitoringStruct
{
   private :
      #if USE_MUTEX
      RT_MUTEX mtx_taskStatus;
      #endif
      RT_TASK xenoTask;

      //bool isExecuted; // Run-time - computed  // will be USELESS //
      uint maxPendingChains;
      uint oldestElement, newestElement;
      taskMonitoringStruct* precedentTask;

      ExecTimes* execLogs; // tableau =new timelog[n]

   public :
      uint id;
      uint precedencyID;

      ChainDataLogger* logger;

      RTIME deadline;    // Static = period
      RTIME rwcet;       // Static
      //bool operator <(const taskMonitoringStruct& tms) const {return (id < tms.id);}

      taskMonitoringStruct(rtTaskInfosStruct rtTaskInfos, ChainDataLogger* logger);
      void setChainInfos(int bufsize, taskMonitoringStruct* prec);
      bool addEntry(ExecTimes times);
      ExecTimes emptyUntil(RTIME limitTime);
      bool emptyPrecedency( RTIME limitTime, RTIME endOfChain); // récursif
      ExecTimes getState(RTIME limit = 0);
      void displayInfos();
};

class taskChain
{
   private:
      RTIME end2endDeadline; // static

      RTIME getExecutionTime();
      RTIME getRemWCET();

   public :
      ChainDataLogger* logger;
      char name[32];
      uint chainID;                // static
      RTIME startTime;       // Runtime - deduced
      //RTIME remWCET;         // Runtime - computed
      bool isAtRisk;         // Runtime - deduced

      // A AJOUTER POUR GERER PLUSIEURS OCCURENCES //
      taskMonitoringStruct* lastTask;
      void setPrecedencies();
      void updateStartTime();
      bool unloadChain(RTIME endOfChain);  // renvoi la date de début de chaine;
      ///////////////////////////////////////////////
      std::vector<taskMonitoringStruct> taskList;

      taskChain(end2endDeadlineStruct _tcDeadline, string outfile);

      //bool checkPrecedency(uint taskID); // devient useless
      bool checkTaskE2E();    // a modifier
      //bool checkIfEnded();    // a modifier // devient USELESS
      void displayTasks();
      //void resetChain();      // a modifier => void unloadChain()
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
      static bool EndOfExpe;
      //RT_MUTEX _bufMtx;
      RT_TASK _task;
      RT_BUFFER _buff;

      TaskProcess(rtTaskInfosStruct _taskInfo);
      virtual void executeRun() = 0;
      void saveData(int nameMaxSize);
};

class MacroTask : public TaskProcess
{
   protected :
      int ret;
      TaskDataLogger* dataLogs;
      int (*proceed_function)(int Argc, char *argv[]);
      void findFunction(char* _func);

   public :
      rtTaskInfosStruct prop;

      MacroTask(rtTaskInfosStruct taskInfos, bool MoCoMode, string name);
      virtual void setCommunications() = 0;
      inline virtual uint before() = 0;
      inline int proceed();
      inline virtual int after() = 0;
      void saveData(int maxNameSize, RT_TASK_INFO* cti = NULL);
};

class RTMacroTask : public MacroTask
{
   public:
      RTMacroTask(rtTaskInfosStruct taskInfos, bool MoCoMode, string name);

      void setCommunications();
      void executeRun();
      inline uint before();
      inline int after();
};

class BEMacroTask : public MacroTask
{
   public:
      BEMacroTask(rtTaskInfosStruct taskInfos, bool MoCoMode, string name);

      void setCommunications();
      void executeRun();
      inline uint before();
      inline int after();
};

class Agent : public TaskProcess
{
   protected :
      RT_TASK msgReceiverTask;
      //bool enable;
      unsigned int runtimeMode;    // NOMINAL or OVERLOADED
      ulong overruns;
      std::vector<taskChain> allTaskChain;
      std::vector<RT_TASK> bestEffortTasks;

      void initCommunications();
      void setAllChains(std::vector<end2endDeadlineStruct> _tcDeadlineStructs);
      void setAllTasks(std::vector<rtTaskInfosStruct> _TasksInfos);
      void setMode(uint mode);
      //int checkTaskChains();
      void displaySystemInfo(std::vector<end2endDeadlineStruct> e2eDD,
                             std::vector<rtTaskInfosStruct> tasksSet);
      void displayChains();

   public :
      Agent(rtTaskInfosStruct _taskInfo,
            std::vector<end2endDeadlineStruct> e2eDD,
            std::vector<rtTaskInfosStruct> tasksSet);

      void updateTaskInfo(monitoringMsg msg);
      void saveData();
      void executeRun();
};

class MonitoringAgent : public Agent
{
   public:
      MonitoringAgent(rtTaskInfosStruct _taskInfo,
                     std::vector<end2endDeadlineStruct> e2eDD,
                     std::vector<rtTaskInfosStruct> tasksSet);

      void executeRun();
      static void messageReceiver(void* _arg /* Agent* */);
      void saveData();

};

class MonitoringControlAgent : public MonitoringAgent
{
   public:
      MonitoringControlAgent(rtTaskInfosStruct _taskInfo,
                              std::vector<end2endDeadlineStruct> e2eDD,
                              std::vector<rtTaskInfosStruct> tasksSet);

      void executeRun();
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

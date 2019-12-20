#ifndef TASKS_H
#define TASKS_H

#include "tools.h"
#include "dataLogger.h"
#include "taskLauncher.h"
#include <algorithm>
#define   MODE_OVERLOADED     1

#define   TRUE    1
#define   FALSE   0
#define   MODE_NOMINAL        -1
#define   MODE_DISABLE        0

const RTIME t_RT = 000*1.0e6;  // time to trigger the Control Agent
const RTIME Wmax = 400*1.0e6;    // next slice max time

class TaskProcess
{
   protected:
      TaskLauncher* tl;
      RT_EVENT	event;
      monitoringMsg msg ;
      char stdIn[35];
      char stdOut[35];
      bool MoCoIsAlive;

      void setIO();
      void setRTtask();
      void setAffinity (int _aff, int mode);

   public:
      RT_TASK _task;
      RT_BUFFER bf;

      //TaskProcess(taskRTInfo* , bool );
      virtual void executeRun();
};

class MacroTask : public TaskProcess
{
  protected :
    /* struct rtTaskInfosStruct
    {
         char   name[32];
         char   path_task[128];
         string arguments;
         int    isHardRealTime;     // task chain ID or best effort if null
         int    id;
         int    affinity;
         int    precedency;
         int    priority;
         RTIME  wcet;
         RTIME  periodicity;
         RT_TASK* task;
    } ;
    */
    rtTaskInfosStruct prop;
    std::vector<char*> _argv;
    TaskDataLogger* dataLogs;
    int (*proceed_function)(int Argc, char *argv[]);

    void setRTtask();
    void parseParameters( );
    inline int before();
    inline void proceed();
    inline int after();
    inline int before_besteff();
    inline int after_besteff();

  public :
    MacroTask(rtTaskInfosStruct, bool);
    ~MacroTask();
    void executeRun();
    void executeRun_besteffort();

};

class taskMonitoringStruct
{
  private :
    RT_MUTEX mtx_taskStatus;
    bool isExecuted; // Run-time - computed

  public :
    taskMonitoringStruct(rtTaskInfosStruct* rtTaskInfos);
    int precedencyID;
    RT_TASK* xenoTask;
    int id;
    //RTIME endTime;     // Run-time - received
    RTIME deadline;    // Static
    RTIME rwcet;       // Static

    void setState(bool state);
    bool getState();
    //bool operator <(const taskMonitoringStruct& tms) const {return (id < tms.id);}
};

class taskChain
{
  public :
    taskChain(end2endDeadlineStruct _tcDeadline);
    ChainDataLogger* logger;
    int cptAnticipatedMisses;
    //int cptOutOfDeadline;
    //int cptExecutions;
    //std::array<RTIME, 2048> chainExecutionTime;
    char name[32];
    int chainID;                // static
    RTIME end2endDeadline; // static
    RTIME startTime;       // Runtime - deduced
    RTIME currentEndTime;  // Runtime - deduced
    RTIME remWCET;         // Runtime - computed
    bool isAtRisk;         // Runtime - deduced
    //double slackTime;
    std::vector<taskMonitoringStruct> taskList;

    bool checkPrecedency(int taskID);
    bool checkTaskE2E();
    bool checkIfEnded();
    void resetChain();
    RTIME getExecutionTime();
    RTIME getRemWCET();

    void displayTasks();
};

class MCAgent : public TaskProcess
{
   protected :
        bool enable;
        short runtimeMode;    // NOMINAL or OVERLOADED
        ulong overruns;
        std::vector<taskChain*> allTaskChain;
        std::vector<RT_TASK*> bestEffortTasks;

        void setRTtask(systemRTInfo* sInfos);
        void initCommunications();
        void setMode(int mode);
        void setAllDeadlines(std::vector<end2endDeadlineStruct> _tcDeadlineStructs);
        void setAllTasks(std::vector<rtTaskInfosStruct> _TasksInfos);
        //int checkTaskChains();
        void displaySystemInfo(systemRTInfo* sInfos);
        void displayChains();
        void saveData( );

   public :
       MCAgent(systemRTInfo* sInfos, bool enable);
       void updateTaskInfo(monitoringMsg msg);
       void executeRun();
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
extern std::streambuf *cinbuf, *coutbuf;
/*
class MacroTaskBestEffort : public MacroTask
{
  protected :
    void executeRun(RT_SEM* mysync);

  public :
    int before();
    int after();
};
*/

//extern void printTaskInfo(rtTaskInfosStruct* task);

#endif

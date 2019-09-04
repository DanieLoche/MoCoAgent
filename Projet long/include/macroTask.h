#include "tools.h"
#include "dataLogger.h"
#include <algorithm>

class MacroTask
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
    rtTaskInfosStruct* properties;
    TaskDataLogger* dataLogs;

    RT_BUFFER bf;
    RT_EVENT	event;
//    RT_MUTEX mutex;
    bool MoCoIsAlive;
    int priority;
    monitoringMsg msg ;
    //string chain;
    int (*proceed_function)(int argc, char *argv[]);
    char stdIn[35];
    char stdOut[35];
    //int argc;
    std::vector<char*> argv;


    void parseParameters( );
    inline int before_besteff();
    inline int before();
    inline void proceed();
    inline int after();
    inline int after_besteff();

  public :
    MacroTask(taskRTInfo*, bool);
    void executeRun();
    void executeRun_besteffort();

};

extern int basicmath_small   (int argc, char *argv[]);
extern int basicmath_large   (int argc, char *argv[]);
extern int bitcount_func     (int argc, char *argv[]);
extern int qsort_small       (int argc, char *argv[]);
extern int qsort_large       (int argc, char *argv[]);
extern int susan             (int argc, char *argv[]);
extern int djpeg_func        (int argc, char *argv[]);
extern int cjpeg_func        (int argc, char *argv[]);
extern int typeset_func      (int argc, char *argv[]);
extern int dijkstra_small    (int argc, char *argv[]);
extern int dijkstra_large    (int argc, char *argv[]);
extern int patricia          (int argc, char *argv[]);
extern int stringsearch_small(int argc, char *argv[]);
extern int stringsearch_large(int argc, char *argv[]);
extern int blowfish          (int argc, char *argv[]);
extern int rijndael          (int argc, char *argv[]);
extern int sha               (int argc, char *argv[]);
extern int rawdaudio         (int argc, char *argv[]);
extern int rawcaudio         (int argc, char *argv[]);
extern int crc               (int argc, char *argv[]);
extern int fft               (int argc, char *argv[]);
extern int gsm_func          (int argc, char *argv[]);

extern void print_affinity(pid_t _pid);
extern void printTaskInfo(rtTaskInfosStruct* task);

extern void print_affinity(pid_t _pid);
extern void printTaskInfo(rtTaskInfosStruct* task);

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

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
    std::ifstream inStrm;
    std::ofstream outStrm;
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

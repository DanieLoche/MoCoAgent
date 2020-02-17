#ifndef TOOLS_H
#define TOOLS_H

#define   TRUE    1
#define   FALSE   0

//define    SCHED_FIFO      1 // First-In First-Out
//define    SCHED_RR        2 // Round-Robin
//define    SCHED_WEAK      0 // Weak
//define    SCHED_COBALT    3 // Cobalt
//define    SCHED_SPORADIC  4 // Sporadic
//define    SCHED_TP        5 // TP
//define    SCHED_QUOTA     8 // Quota
#define     SCHED_EDF       6 // Not Implemented
#define     SCHED_RM        7 // Rate-Monotonic

#define   RR_SLICE_TIME     20e9  // clock ticks (=ns)
#define   SPINTIME          1e7   // spin time in ns
#define   EXECTIME          2e8   // execution time in ns


#define   VERBOSE_INFO      1 // Cout d'informations, démarrage, etc...
#define   VERBOSE_DEBUG     1 // Cout de débug...
#define   VERBOSE_OTHER     1 // Cout autre...
#define   VERBOSE_ASK       1 // cout explicitement demandés dans le code

#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <unistd.h>
#include <signal.h>
#include <cstdlib>

#include <alchemy/task.h>
#include <alchemy/sem.h>
#include <alchemy/timer.h>
#include <alchemy/buffer.h>
#include <alchemy/alarm.h>
#include <alchemy/event.h>
#include <alchemy/mutex.h>

using std::string;
using std::cout;
using std::endl;
using std::cin;
using std::cerr;

#define Sec2T(_time)  rt_timer_ns2ticks((_time)*1e9)
#define mS2T(_time)   rt_timer_ns2ticks((_time)*1e6)
#define uS2T(_time)   rt_timer_ns2ticks((_time)*1e3)
#define nS2T(_time)   rt_timer_ns2ticks(_time)

#define ERROR_MNG(fct)                                                 \
do {                                                                   \
   int err = fct;                                                      \
   if ( err != 0)                                                      \
   {                                                                   \
      rt_fprintf(stderr, "%s-%s error %d\n", __FUNCTION__, #fct, err); \
      exit(EXIT_FAILURE);                                              \
   }                                                                   \
} while(0)

//#define TO_STRING(str) convertToString(str)

struct rtPStruct // Real-time Parameters
{
   //RT_TASK* _t;  //

   int affinity; //
   RTIME periodicity; //
   int priority;      //
   int schedPolicy;   //

};

struct funcPStruct   // Functional parameters
{
   int id;
   char name[32]; //
   char func[128];
   string args;
   int isHRT;     // task chain ID or best effort if null
   int prec;
   RTIME wcet;   //
};

struct rtTaskInfosStruct
{
   rtPStruct rtP;
   funcPStruct fP;
};
/*
struct logData
{
  struct timeLog
  {
    RTIME timestamp;
    RTIME duration;
  } timeLogs[8000];
  int cptOutOfDeadline;
  int cptExecutions;
};
*/
/*struct rtTaskInfosStruct
{
   RT_TASK* _t;  //
   char name[32]; //
   char function[128];
   string arguments;
   int isHardRealTime;     // task chain ID or best effort if null
   int id;
   int precedency;
   RTIME wcet;   //
   RTIME periodicity; //
   int affinity; //
   int priority;      //
   int schedPolicy;   //
};*/

struct sortAscendingPeriod {
   inline bool operator() (const rtTaskInfosStruct& struct1, const rtTaskInfosStruct& struct2)
   {
      return (struct1.rtP.periodicity < struct2.rtP.periodicity);
   }
};
struct sortDescendingPeriod {
   inline bool operator() (const rtTaskInfosStruct& struct1, const rtTaskInfosStruct& struct2)
   {
      return (struct1.rtP.periodicity > struct2.rtP.periodicity);
   }
};

struct end2endDeadlineStruct
{
  char name[32];
  int taskChainID;
  string Path;
  RTIME deadline;
};

// struct systemRTInfo
// {
//   std::vector<end2endDeadlineStruct> e2eDD;
//   std::vector<rtTaskInfosStruct> rtTIs;
//   //bool* triggerSave;
//   //string outputFileName;
// };

struct monitoringMsg
{
  RT_TASK* task;
  int ID;
  RTIME time;   // Run-time - received
  bool isExecuted;    // Run-time - computed
};


std::string trim(const std::string& str,
                 const std::string& whitespace = " \t");

std::string reduce(const std::string& str,
                   const std::string& fill = " ",
                   const std::string& whitespace = " \t");

//string convertToString(const char* a){ std::string s = a; return s; }

void printInquireInfo(RT_TASK*);
void printTaskInfo(rtTaskInfosStruct* task);
void print_affinity(pid_t _pid);


/* To create a task :
 * Arguments : &task,
 *             name,
 *             stack size (def = 0)
 *             priority
 *             mode (FPU, Start suspended, ...)
 */

/* To start a task
* Arguments : &task,
*             task main
*             function Arguments
*/

/* RT_TASK_INFO structure
* returned by a call to rt_task_inquire(task, &rti).
* int 	                  prio
* struct threadobj_stat 	stat
* char 	                  name
* pid_t 	                pid
*/

/* threadobj_stat structure
 ticks_t xtime;     => Total execution time in primary mode (ns)
 ticks_t timeout;   => Pending timeout (ns)
 uint64_t msw;      => Total count of mode switches (primary -> secondary)
 uint64_t csw;      => Total count of context switches
 uint64_t xsc;      => Total count of Cobalt system calls.
 int cpu;           => Current CPU.
 int schedlock;     => Scheduler lock depth count.
 unsigned int status; => Internal status bits (from uapi/kernel/thread.h, state flags)
 uint32_t pf;       => Number of page faults in primary mode. Debug stuff, should be zero.
*/


#endif

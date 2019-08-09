#ifndef TOOLS_H
#define TOOLS_H

//define    SCHED_FIFO      1 // First-In First-Out
//define    SCHED_RR        2 // Round-Robin
//define    SCHED_WEAK      0 // Weak
//define    SCHED_COBALT    3 // Cobalt
//define    SCHED_SPORADIC  4 // Sporadic
//define    SCHED_TP        5 // TP
//define    SCHED_QUOTA     8 // Quota
#define     SCHED_EDF       6 // Not Implemented
#define     SCHED_RM        7 // Rate-Monotonic

#define   SCHED_POLICY      SCHED_FIFO
#define   RR_SLICE_TIME     20000e6  // clock ticks (=ns)

#define   VERBOSE_INFO      1 // Cout d'informations, démarrage, etc...
#define   VERBOSE_DEBUG     1 // Cout de débug...
#define   VERBOSE_OTHER     0 // Cout autre...
#define   VERBOSE_ASK       0 // cout explicitement demandés dans le code

#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <unistd.h>
#include <signal.h>
#include <cstdlib>

#include <cobalt/pthread.h>
#include <cobalt/semaphore.h>
#include <cobalt/time.h>
#include <cobalt/mqueue.h>
#include <cobalt/signal.h>
//#include <cobalt/mutex.h>


using std::string;
using std::cout;
using std::endl;
using std::cin;

void setTime( uint64_t time_us /* in micro ! */, timespec* timeOut) {
   timeOut->tv_sec = time_us / 1000000;
   timeOut->tv_nsec = (time_us - 1000000*timeOut->tv_sec) * 1000;
}

void cpyTime( timespec* timeIn, timespec* timeOut ) {
   timeOut->tv_sec = timeIn->tv_sec;
   timeOut->tv_nsec = timeIn->tv_nsec;
}

timespec diffTime( timespec timeA, timespec timeB )
{
   timespec timeDiff;
   timeDiff.tv_sec = labs(timeA.tv_sec - timeB.tv_sec);
   timeDiff.tv_nsec = labs(timeA.tv_nsec - timeB.tv_nsec);
   return timeDiff;
}

uint64_t getTime_us(timespec tp){
   return (uint64_t) (tp.tv_sec * 1000000 + tp.tv_nsec / 1000); // time in microseconds
}

uint64_t getTime_ms(timespec tp){
      return (uint64_t) (tp.tv_sec * 1000 + tp.tv_nsec / 1000000); // time in milliseconds
}

inline int getTime(timespec* tp){
   if (clock_gettime(CLOCK_MONOTONIC, tp) == 0)
      return 0; // time in milliseconds
   else return -1;
}

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
struct rtTaskInfosStruct
{
   char name[32];
   char path_task[128];
   string arguments;
   int isHardRealTime;     // task chain ID or best effort if null
   int id;
   int affinity;
   int precedency;
   int priority;
   uint64_t wcet; // in milliseconds
   uint64_t periodicity; // in milliseconds

   struct xnthread* task;
};

struct sortAscendingPeriod {
   inline bool operator() (const rtTaskInfosStruct& struct1, const rtTaskInfosStruct& struct2)
   {
      return (struct1.periodicity < struct2.periodicity);
   }
};
struct sortDescendingPeriod {
   inline bool operator() (const rtTaskInfosStruct& struct1, const rtTaskInfosStruct& struct2)
   {
      return (struct1.periodicity > struct2.periodicity);
   }
};

struct end2endDeadlineStruct
{
  char name[32];
  int taskChainID;
  string Path;
  uint64_t deadline; // in milliseconds
};

struct monitoringMsg
{
  xnthread* task;
  int ID;
  timespec time;   // Run-time - received
  bool isExecuted;    // Run-time - computed
};


struct systemRTInfo
{
  std::vector<end2endDeadlineStruct> e2eDD;
  std::vector<rtTaskInfosStruct> rtTIs;
  bool* triggerSave;
  string outputFileName;
};

std::string trim(const std::string& str,
                 const std::string& whitespace = " \t");

std::string reduce(const std::string& str,
                   const std::string& fill = " ",
                   const std::string& whitespace = " \t");

void printInquireInfo(xnthread*);
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

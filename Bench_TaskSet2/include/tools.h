#ifndef TOOLS_H
#define TOOLS_H

//#define PRINT 1

#include <fstream>
#include <sstream>
#include <iostream>
#include <stdio.h>
#include <string>
#include <vector>
#include <algorithm>
#include <unistd.h>
#include <signal.h>
#include <cstdlib>

#include <alchemy/task.h>
#include <alchemy/sem.h>
#include <alchemy/timer.h>
#include <sys/time.h>
#include <sys/resource.h>

using std::string;
using std::cout;
using std::endl;
using std::cin;

struct rtTaskInfosStruct
{
    RT_TASK* task;
    char   name[64];
    string path;
    string task_args;

    bool isHardRealTime;
    int  periodicity;
    RTIME  deadline;
    int  affinity;
    RTIME average_runtime;
    RTIME max_runtime;
    RTIME min_runtime;
    int  out_deadline;
    int  num_of_times;
    bool Exectued ;
    int ChaineID;

} ;

struct ChaineInfo_Struct
{
  string name ;
  int ChaineID;
  int Num_tasks;
  int Path;
  double Deadline;
  RTIME WCET;
  RTIME Wmax ;
  RTIME Excution_time ;


};


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
 uint
 int cpu;           => Current CPU.
 int schedlock;     => Scheduler lock depth count.
 unsigned int status; => Internal status bits (from uapi/kernel/thread.h, state flags)
 uint32_t pf;       => Number of page faults in primary mode. Debug stuff, should be zero.
*/


#endif
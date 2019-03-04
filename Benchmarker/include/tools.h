#ifndef TOOLS_H
#define TOOLS_H

#include <fstream>
#include <sstream>
#include <iostream>
#include <stdio.h>
#include <string>
#include <vector>
#include <algorithm>
#include "sched.h"
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <alchemy/task.h>

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
    int  deadline;
    int  affinity;
} ;

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

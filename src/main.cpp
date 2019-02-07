#define VERBOSE_INFO  1 // Cout d'informations, démarrage, etc...
#define VERBOSE_DEBUG 1 // Cout de débug...
#define VERBOSE_OTHER 1 // Cout autre...
#define VERBOSE_ASK   1 // cout explicitement demandés dans le code

#include "tools.h"
#include <sys/sysinfo.h>
#include "sched.h"

#include "mcAgent.h"
#include "macroTask.h"
#include "taskLauncher.h"


long nproc;

void printTaskInfo(rtTaskInfosStruct* task)
{
  #if VERBOSE_INFO
  cout << "Name: " << task->name
      << "| path: " << task->path
      << "| is RT ? " << task->isHardRealTime
      << "| Period: " << task->periodicity
      << "| Deadline: " << task->deadline
      << "| affinity: " << task->affinity << endl;
  #endif
}

void print_affinity(pid_t _pid)
{
 #if VERBOSE_INFO
    cpu_set_t mask;
    long i;

    if (sched_getaffinity(_pid, sizeof(cpu_set_t), &mask) == -1) {
        perror("sched_getaffinity");
        assert(false);
    } else {
        RT_TASK_INFO curtaskinfo;
        rt_task_inquire(NULL, &curtaskinfo);
        cout << "Affinity of thread " << curtaskinfo.pid << " = ";
        for (i = 0; i < nproc; i++)
            cout << CPU_ISSET(i, &mask);
        cout << endl;
        /* using printf
        printf("sched_getaffinity = ");
        for (i = 0; i < nproc; i++) {
            printf("%d ", CPU_ISSET(i, &mask));
        }
        printf("\n");
        */
    }
 #endif

}

void RunmcAgentMain(void* arg)
{
  MCAgent mcAgent(arg);
  RT_TASK_INFO curtaskinfo;

  rt_task_inquire(NULL, &curtaskinfo);
  #if VERBOSE_INFO
    cout << curtaskinfo.pid << " : " << "executed in primary for " << curtaskinfo.stat.xtime << " ns" << endl;
  #endif

}


void TaskMain(void* arg)
{
  rtTaskInfosStruct* rtTI = (rtTaskInfosStruct*) arg;

  RT_TASK_INFO curtaskinfo;
  rt_task_inquire(NULL, &curtaskinfo);
  #if VERBOSE_INFO
    cout << "I am task : " << curtaskinfo.name << " of priority " << curtaskinfo.prio << endl;
  #endif

  MacroTask macroRT;
  macroRT.properties = *rtTI;
  macroRT.executeRun();

}

int main(int argc, char* argv[])
{
  int return_code = 0;
  nproc = get_nprocs();
  // get input file, either indicated by user as argument or default location
  string input_file;
  if (argc > 1) input_file = argv[1];
  else input_file = "./input.txt";

  TaskLauncher tln(input_file);
  //tln.tasksInfos = readTasksList(input_file);

  tln.printTasksInfos();
  tln.runTasks();


  return return_code;
}

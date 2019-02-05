#include "tools.h"
#include <sys/sysinfo.h>
#include "sched.h"

#include "mcAgent.h"
#include "macroTask.h"
#include "taskLauncher.h"

long nproc;
RT_SEM mysync;

void printTaskInfo(rtTaskInfosStruct* task)
{
  cout << "Name: " << task->name
      << "| path: " << task->path
      << "| is RT ? " << task->isHardRealTime
      << "| Period: " << task->periodicity
      << "| Deadline: " << task->deadline
      << "| affinity: " << task->affinity << endl;
}

void print_affinity(pid_t _pid)
{
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

}

void RunmcAgentMain(void* arg)
{
  cout << "MoCoAgent running ?!" << endl;
  MCAgent mcAgent(arg);
}


void TaskMain(void* arg)
{
  rtTaskInfosStruct* rtTI = (rtTaskInfosStruct*) arg;

  RT_TASK_INFO curtaskinfo;
  rt_task_inquire(NULL, &curtaskinfo);
  cout << "I am task : " << curtaskinfo.name << " of priority " << curtaskinfo.prio << endl;

  MacroTask macroRT;
  macroRT.properties = *rtTI;
  macroRT.executeRun(&mysync);

}

int main(int argc, char* argv[])
{
  rt_sem_create(&mysync,"MySemaphore",0,S_FIFO);
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
  usleep(1000000);
  cout<<"wake up all tasks\n"<<endl;
  rt_sem_broadcast(&mysync);

  //printf("\nType CTRL-C to end this program\n\n" );
  pause();

  return return_code;
}

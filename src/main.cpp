#include "tools.h"
#include <sys/sysinfo.h>
#include "sched.h"

#include "mcAgent.h"
#include "macroTask.h"
#include "taskLauncher.h"
#include "buildSet.h"


#define EXECTIME   2e8   // execution time in ns
#define SPINTIME   1e7   // spin time in ns



long nproc;
RT_SEM mysync;
TaskLauncher* tasl;

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

  //cout << "I am task : " << curtaskinfo.name << " of priority " << curtaskinfo.prio << endl;

  MacroTask macroRT;
  macroRT.properties = rtTI;
  macroRT.executeRun(&mysync);

}


void my_handler(int s){

  for (auto taskInfo = tasl->tasksInfosList.begin(); taskInfo != tasl->tasksInfosList.end(); ++taskInfo)
  {
           printf("Caught signal %s\n",taskInfo->name);
           printf("Average runtime %f ms\n",taskInfo->average_runtime/ 1.0e6);
           printf("Max runtime %f ms\n",taskInfo->max_runtime/ 1.0e6);
           printf("Min runtime %f ms\n",taskInfo->min_runtime/ 1.0e6);
           printf(" dead_line  %f ms \n",taskInfo->deadline / 1.0e6);
           printf("Out of dead_line  %d\n",taskInfo->out_deadline);

  }
   exit(1);
}


int main(int argc, char* argv[])
{
  // POUR TEST EN ATTENDANT LA LISTE D'ENTREE
  /*std::vector<string> long_task;
  std::vector<string> short_task;

  for (int i = 0; i<10; i++) {
    std::string l = "exe" + std::to_string(i+1) + "L";
    std::string s = "exe" + std::to_string(i+1) + "S";
    long_task.push_back(l);
    short_task.push_back(s);
  }*/


  rt_sem_create(&mysync,"MySemaphore",0,S_FIFO);

  //rt_task_set_mode(0,XNRRB,NULL);

  int return_code = 0;
  nproc = get_nprocs();
  // get input file, either indicated by user as argument or default location
  string input_file;
  string task_file;

  if (argc > 1) input_file = argv[1];
  else input_file = "./input.txt";
  if (argc > 2) task_file = argv[2];
  else task_file = "./sorted.txt";

  buildSet bS;

  // Définition des listes comportant les tâches longue et courte
  std::vector<string> all_crit_tasks = bS.distributionCrit(1, 1, 50);

  // Définition des tâches non critiques choisies
  std::vector<string> uncrit_tasks = bS.get_uncrit_tasks();

  // Récupération infos tâches
  std::vector<rtTaskInfosStruct> info_task = bS.get_infos_tasks(task_file);

  // Edition du fichier input.txt
  bS.buildInput();


  TaskLauncher tln(input_file);
  //tln.tasksInfos = readTasksList(input_file);
  tln.printTasksInfos();
  tln.runTasks();
  tasl=&tln;

  //sleeping the time that all tasks will be started
  usleep(1000000);
  cout<<"wake up all tasks\n"<<endl;
  rt_sem_broadcast(&mysync);

  printf("\nType CTRL-C to end this program\n\n" );

  struct sigaction sigIntHandler;

  sigIntHandler.sa_handler = my_handler;;
  sigemptyset(&sigIntHandler.sa_mask);
  sigIntHandler.sa_flags = 0;

  sigaction(SIGINT, &sigIntHandler, NULL);

  pause();



/*  sleep(1);

  cout<<"stop up all tasks\n"<<endl;
  for (auto taskInfo = tasl->tasksInfosList.begin(); taskInfo != tasl->tasksInfosList.end(); ++taskInfo)
  {

           RT_TASK_INFO curtaskinfo;
           rt_task_inquire(taskInfo->task, &curtaskinfo);
           kill(curtaskinfo.pid,SIGINT);
           printf("Caught signal killed %s\n",taskInfo->name);
    }
   for (auto taskInfo = tasl->tasksInfosList.begin(); taskInfo != tasl->tasksInfosList.end(); ++taskInfo)
   {
           printf("Caught signal %s\n",taskInfo->name);
           printf("Average runtime %f ms\n",taskInfo->average_runtime/ 1.0e6);
           printf("Max runtime %f ms\n",taskInfo->max_runtime/ 1.0e6);
           printf("Min runtime %f ms\n",taskInfo->min_runtime/ 1.0e6);
           printf(" dead_line  %f ms \n",taskInfo->deadline / 1.0e6);
           printf("Out of dead_line  %d\n",taskInfo->out_deadline);

  }
   exit(0);*/


  return return_code;
}

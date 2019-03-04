#include "tools.h"
#include <sys/sysinfo.h>


#include "macroTask.h"
#include "taskLauncher.h"

#define EXECTIME   2e8   // execution time in ns
#define SPINTIME   1e7   // spin time in ns
#define TARGET 100;

long nproc;
float sum = 0.0;
pid_t pid;

TaskLauncher* tasl;

string output_file;
long cmp;
float total_task;

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

    if (sched_getaffinity(_pid, sizeof(cpu_set_t), &mask) == -1) {
        perror("sched_getaffinity");
        assert(false);
    } else {
        RT_TASK_INFO curtaskinfo;
        rt_task_inquire(NULL, &curtaskinfo);
    }

}

void TaskMain(void* arg)
{
  /*
  output_file = "res.txt";
  std::ofstream myfile;
  myfile.open (output_file,std::ios_base::app);
  */
  output_file = "result4.txt";
  std::ofstream myfile;
  myfile.open (output_file,std::ios_base::app);

  rtTaskInfosStruct* rtTI = (rtTaskInfosStruct*) arg;
  double exec_time;
  RT_TASK_INFO curtaskinfo;
  rt_task_inquire(NULL, &curtaskinfo);
  MacroTask *macroRT = new MacroTask;

  macroRT->properties = rtTI;
 //rt_sem_p(&mysync,TM_INFINITE);

  macroRT->executeRun();

  usleep(1000000);
  cout << "Task " << macroRT->properties->name << " runtime: " << macroRT->properties->average_runtime/1e6 << " ms" << endl;
  myfile << "Task " << macroRT->properties->name << " runtime: " << macroRT->properties->average_runtime/1e6 << " ms" << endl;

  myfile.close();

  exec_time =  macroRT->properties->average_runtime/1e6;
  total_task += exec_time;
  cmp += 1;
  kill(curtaskinfo.pid, SIGINT);
}



int main(int argc, char *argv[])
{

  output_file = "result4.txt";
  std::ofstream myfile;
  system("clear");
  string input_file = "./input4.txt";
  rt_sem_create(&mysync,"MySemaphore",0,S_FIFO);


  int return_code = 0;
  nproc = get_nprocs();

  int target = TARGET;
  TaskLauncher tln(input_file);
  tln.printTasksInfos();

 for (int j = 1 ; j<target+1 ; j++){

   total_task=0;
   cmp=0;
   output_file = "result4.txt";
   myfile.open (output_file,std::ios_base::app);
     myfile << endl << "Iteration " << j << " sur " << target << endl;
     myfile.close();
     cout << endl << "Iteration " << j << " sur " << target << endl;

  tln.runTasks();
  usleep(1000000);

  rt_sem_broadcast(&mysync);

  sleep(1);
  myfile.open (output_file,std::ios_base::app);
    myfile << "Temps moy : " << total_task/cmp << "ms"<<endl;
    myfile.close();

  cout << "Temps moy : " << total_task/cmp << "ms"<<endl;

      sum+=total_task;

 }
  sum = (sum / target);

  myfile.open (output_file,std::ios_base::app);
  myfile << "---------------------------------"<<endl;
  myfile << "Temps moy total: " << sum << "ms"<<endl;
  myfile << "Done Bench" << endl;
  myfile.close();

  return return_code;
}

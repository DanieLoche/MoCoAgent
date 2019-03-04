#include "tools.h"
#include <sys/sysinfo.h>

#include "macroTask.h"
#include "taskLauncher.h"

#define TARGET 100;
#define TASKNUM 30;
#define OUTPUT_FILE "Bench_Aquarius.txt";
#define MEMORY 100000000;
using namespace std;
long nproc;
double sum = 0.0;
double minimum = 10000000.0;
double maximum = 0.0;
pid_t pid;

void printTaskInfo(rtTaskInfosStruct* task)
{
  cout << "Name: " << task->name
      << "| path: " << task->path
      << "| is RT ? " << task->isHardRealTime
      << "| Period: " << task->periodicity
      << "| Deadline: " << task->deadline
      << "| affinity: " << task->affinity << endl; cout.flush();
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
        //cout << "Affinity of thread " << curtaskinfo.pid << " = ";
        //for (i = 0; i < nproc; i++)
        //    cout << CPU_ISSET(i, &mask);
        //cout << endl; cout.flush();
    }
}

void TaskMain(void* arg)
{
  rtTaskInfosStruct* rtTI = (rtTaskInfosStruct*) arg;
  double exec_time;
  RT_TASK_INFO curtaskinfo;
  rt_task_inquire(NULL, &curtaskinfo);

  //cout << "I am task : " << curtaskinfo.name << " of priority " << curtaskinfo.prio << endl; cout.flush();
  MacroTask *macroRT = new MacroTask;

  macroRT->properties = rtTI;

  macroRT->executeRun();
  exec_time = macroRT->get_execution_time();
  sum += exec_time;
  if (exec_time<minimum) minimum = exec_time;
  if (exec_time > maximum) maximum = exec_time;
  kill(curtaskinfo.pid, SIGINT);
}

int main(int argc, char *argv[])
{
  int return_code = 0;
  nproc = get_nprocs();
  pid = getpid();
  // get input file, either indicated by user as argument or default location
  string input_file;
  string out_file = OUTPUT_FILE;
  ofstream myfile;
  myfile.open (out_file);
  myfile << "Task_num Min_exec Max_exec Mean_exec\n";

  myfile.close();
  if (argc > 1) input_file = argv[1];
  else input_file = "./input.txt";
  int j;
  int i;
  int iteration = 2;
  int target = TARGET;
  int task_num = TASKNUM;
  for (j = 0 ; j<task_num ; j++){
    TaskLauncher launcher(input_file,iteration);
    iteration+=1;
    //launcher.printTasksInfos();
    for (i = 0 ; i<target ; i++){
        cout << "Running... " << i << " out of " << target << endl;
        launcher.runTasks();
        sleep(2);
    }
    sum = (sum / target);
    myfile.open (out_file,ios_base::app);
    //myfile << "Tâche ";
    myfile << iteration-2;
    myfile << " ";
    //myfile << "Temps d'exécution min : ";
    myfile << minimum;
    myfile << " ";
    //myfile << " ms\n";
    //myfile << "Temps d'exécution max : ";
    myfile << maximum;
    myfile << " ";
    //myfile << " ms\n";
    //myfile << "Temps d'exécution moyen : ";
    myfile << sum;
    myfile << "\n";
    //myfile << " ms\n\n----------------------------\n\n";
    myfile.close();
    minimum = 10000000.0;
    maximum = 0.0;
    sum = 0.0;
  }

  return return_code;
}

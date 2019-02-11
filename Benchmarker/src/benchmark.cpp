#include "tools.h"
#include <sys/sysinfo.h>


#include "mcAgent.h"
#include "macroTask.h"
#include "taskLauncher.h"

#define TARGET 10;
#define OUTPUT_FILE "results.txt";

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
        cout << endl; cout.flush();
    }
}

void TaskMain(void* arg)
{
  rtTaskInfosStruct* rtTI = (rtTaskInfosStruct*) arg;
  double exec_time;
  RT_TASK_INFO curtaskinfo;
  rt_task_inquire(NULL, &curtaskinfo);

  cout << "I am task : " << curtaskinfo.name << " of priority " << curtaskinfo.prio << endl; cout.flush();
  MacroTask *macroRT = new MacroTask;

  macroRT->properties = *rtTI;
  macroRT->set_main_pid(pid);

  macroRT->executeRun();
  exec_time = macroRT->get_execution_time();
  sum += exec_time;
  if (exec_time<minimum) minimum = exec_time;
  if (exec_time > maximum) maximum = exec_time;
  kill(curtaskinfo.pid, SIGINT);
}

void RunmcAgentMain(void* arg)
{
  cout << "MoCoAgent running ?!" << endl; cout.flush();
  MCAgent mcAgent(arg);
}

int main(int argc, char *argv[])
{
  char tache[64];
  int return_code = 0;
  nproc = get_nprocs();
  pid = getpid();
  // get input file, either indicated by user as argument or default location
  string input_file;
  string out_file = OUTPUT_FILE;
  ofstream myfile;
  myfile.open (out_file,ios_base::app);
  if (argc > 1) input_file = argv[1];
  else input_file = "./input.txt";
  TaskLauncher launcher(input_file);
  launcher.printTasksInfos();
  RT_TASK_INFO curtaskinfo;
  int i;
  int target = TARGET;
  for (i = 0 ; i<target ; i++){
    for (auto& taskInfo : launcher.tasksInfosList)  {
      RT_TASK* task = new RT_TASK;
      taskInfo.task = task;
      rt_task_create(task, taskInfo.name, 90000000, 50, 0);
      cout << "Task " << taskInfo.name << " created." << endl; cout.flush();
      launcher.set_affinity(task, 0);
      cout << "Launching task " << taskInfo.name << " ..." << endl; cout.flush();
      usleep(500);
      cout << "Task " << taskInfo.name << " started." << endl; cout.flush();
      int rep = rt_task_start(taskInfo.task, TaskMain, &taskInfo);
      sleep(2);
      cout << "Task " << taskInfo.name << " stopped." << endl; cout.flush();
      strcpy(tache,taskInfo.name);
      int delrep = rt_task_delete(taskInfo.task);
    }
  }
  sum = (sum / target);
  myfile << "Tâche ";
  myfile << tache;
  myfile << " \n";
  myfile << "Temps d'exécution min : ";
  myfile << minimum;
  myfile << " ms\n";
  myfile << "Temps d'exécution max : ";
  myfile << maximum;
  myfile << " ms\n";
  myfile << "Temps d'exécution moyen : ";
  myfile << sum;
  myfile << " ms\n\n----------------------------\n\n";
  myfile.close();
  return return_code;
}

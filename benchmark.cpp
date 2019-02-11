#include "tools.h"
#include <sys/sysinfo.h>


#include "mcAgent.h"
#include "macroTask.h"
#include "taskLauncher.h"

#define TARGET 10;

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
  cout << "EINVAL = " << EINVAL << endl;
  cout << "EPERM = " << EPERM << endl;
  cout << "EIDRM = " << EIDRM << endl;
  int rep = rt_task_inquire(NULL, &curtaskinfo);
  cout << "Rep = " << rep << endl;
  cout << "I am task : " << curtaskinfo.name << " of priority " << curtaskinfo.prio << endl; cout.flush();
  MacroTask macroRT;
  macroRT.properties = *rtTI;
  macroRT.set_main_pid(pid);
  macroRT.executeRun();
  exec_time = macroRT.get_execution_time();
  sum += exec_time;
  if (exec_time<minimum)
  {
    minimum = exec_time;
  }
  if (exec_time > maximum)
  {
    maximum = exec_time;
  }

}

void handler(int signal, TaskLauncher laucher)
{
    cout << signal << "SIGUSR1 -> Execution terminated" << endl; cout.flush();
}

void RunmcAgentMain(void* arg)
{
  cout << "MoCoAgent running ?!" << endl; cout.flush();
  MCAgent mcAgent(arg);
}

int main(int argc, char *argv[])
{
  int return_code = 0;
  nproc = get_nprocs();
  pid = getpid();
  cout << "I'm main, PID " << pid << "." << endl;
  // get input file, either indicated by user as argument or default location
  string input_file;
  if (argc > 1) input_file = argv[1];
  else input_file = "./input.txt";

  TaskLauncher launcher(input_file);
  launcher.printTasksInfos();
  RT_TASK_INFO curtaskinfo;
  int i;
  int target = TARGET;
  for (i = 0 ; i<target ; i++){
    launcher.runTasks();
    sleep(1);
  }
  sum = (sum / target);
  cout << "Temps d'exécution min : " << minimum << " ms" << endl; cout.flush();
  cout << "Temps d'exécution max : " << maximum << " ms" << endl; cout.flush();
  cout << "Temps d'exécution moyen : " << sum << " ms" << endl; cout.flush();

  return return_code;
}

#include "tools.h"
#include <sys/sysinfo.h>
#include "sched.h"

#include "mcAgent.h"
#include "macroTask.h"
#include "taskLauncher.h"
//#include "buildSet.h"
#include <mutex>

#define EXECTIME   2e8   // execution time in ns
#define SPINTIME   1e7   // spin time in ns
#define TARGET 100
long nproc;
int total_task;
float sum;
RT_SEM mysync;
std::vector<ChaineInfo_Struct> list_info_chaine;

TaskLauncher* tasl;
std::mutex mutex;           // mutex for critical section

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
        #ifdef PRINT
        cout << "Affinity of thread " << curtaskinfo.pid << " = ";
        for (i = 0; i < nproc; i++)
            cout << CPU_ISSET(i, &mask);
        cout << endl;
        #endif
    }

}

void RunmcAgentMain(void* arg)
{
  cout << "MoCoAgent running ?!" << endl;
  MCAgent mcAgent(arg);
}

string output_file;
void TaskMain(void* arg)
{
  output_file = "res.txt";
  std::ofstream myfile;
  myfile.open (output_file,std::ios_base::app);
  rtTaskInfosStruct* rtTI = (rtTaskInfosStruct*) arg;
  RT_TASK_INFO curtaskinfo;
  rt_task_inquire(NULL, &curtaskinfo);
  MacroTask macroRT;
  macroRT.properties = rtTI;
  macroRT.executeRun(&mysync);
  cout << "Task " << macroRT.properties->name << " av. runtime " << macroRT.properties->average_runtime/1e6 << " ms" << endl;
  myfile << macroRT.properties->name << " " << curtaskinfo.stat.csw << " " << curtaskinfo.stat.xsc << " " << curtaskinfo.stat.xtime << " " << macroRT.usage.ru_ixrss << " " << macroRT.usage.ru_idrss << "\n";
  int time = curtaskinfo.stat.xtime;
  total_task += time;
  cout << time << endl;
  myfile.close();
  #ifdef PRINT
  cout << "Data saved for task" << macroRT.properties->name << endl;
  #endif
}


int main(int argc, char* argv[])
{
  output_file = "res.txt";
  std::ofstream myfile;
  myfile.open (output_file);
  myfile << "Name Context_switches System_Calls Total_Execution_Time Shared_Mem Non_Shared_Mem\n";
  myfile.close();
  system("clear");
  string input_file = "./input1.txt";

  int return_code = 0;
  nproc = get_nprocs();

  int target = TARGET;

  TaskLauncher tln(input_file);
  tln.printTasksInfos();

  for (int j=0; j < target; j++){
    total_task =0;
    //for(int i=0; i < (int)list_info_chaine.size();i++ ){
    rt_sem_create(&mysync,"MySemaphore",0,S_FIFO);

      cout << endl << "Itération " << j << " sur " << target << endl;

      tln.runTasks();
      //tasl=&tln;

    //}
    //sleeping the time that all tasks will be started
    usleep(1000000);
    #ifdef PRINT
    cout<<"wake up all tasks\n"<<endl;
    #endif
    rt_sem_broadcast(&mysync);
    cout << "Temps moy : " << total_task/1e3 << "ms"<<endl;

    sleep(1);
    sum+=total_task;
  }

  sum = sum / target;
  cout << "Temps moy : " << sum/1e3 << "ms"<<endl;
  cout << "Done Bench" << endl;
  return return_code;
}

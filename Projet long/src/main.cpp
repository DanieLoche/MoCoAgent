// #define VERBOSE_INFO  1 // Cout d'informations, démarrage, etc...
// #define VERBOSE_DEBUG 0 // Cout de débug...
// #define VERBOSE_OTHER 1 // Cout autre...
// #define VERBOSE_ASK   1 // cout explicitement demandés dans le code


#include "tools.h"
#include <sys/sysinfo.h>
#include "sched.h"

#include "mcAgent.h"
#include "macroTask.h"
#include "taskLauncher.h"
#include "buildSet.h"
#include <mutex>

#define EXECTIME   2e8   // execution time in ns
#define SPINTIME   1e7   // spin time in ns
#define PRINT 1

long nproc;
RT_SEM mysync;
std::vector<end2endDeadlineStruct> list_info_chaine;
std::vector<rtTaskInfosStruct> AlltasksInfosList;

TaskLauncher* tasl;
std::mutex mutex;           // mutex for critical section

void RunmcAgentMain(void* arg)
{
  MCAgent mcAgent(arg);

  printInquireInfo();

}


void TaskMain(void* arg)
{
  rtTaskInfosStruct* rtTI = (rtTaskInfosStruct*) arg;

  RT_TASK_INFO curtaskinfo;
  rt_task_inquire(NULL, &curtaskinfo);
  print_affinity(curtaskinfo.pid);
  #if VERBOSE_OTHER
  cout << "I am task : " << curtaskinfo.name << " of priority " << curtaskinfo.prio << endl;
  #endif
  printTaskInfo(rtTI);

  MacroTask macroRT;
  macroRT.properties = rtTI;

  if ((*rtTI).isHardRealTime == 0) {
    macroRT.executeRun_besteffort(&mysync);
  }
  else {
    macroRT.executeRun(&mysync);
  }


}


void my_handler(int s){
  #if PRINT
  cout << "\n------------------------------" << endl;
  #endif
  string out_file = "Testoutput.txt";
  mutex.lock();
  std::ofstream myfile;
  myfile.open (out_file);    // TO APPEND :  //,ios_base::app);
  for (auto taskInfo = tasl->tasksInfosList.begin(); taskInfo != tasl->tasksInfosList.end(); ++taskInfo)
  {

           myfile << "\nRunning summary for task " << taskInfo->name << ".\n";
           myfile << "Average runtime : " << taskInfo->average_runtime/ 1.0e6 << "\n";
           myfile << "Max runtime : " << taskInfo->max_runtime/ 1.0e6 << " ms\n";
           myfile << "Min runtime : " << taskInfo->min_runtime/ 1.0e6 << " ms\n";
           myfile << "Deadline :" << taskInfo->deadline << " ms\n";
           myfile << "Out of Deadline : " << taskInfo->out_deadline << " times\n";
           myfile << "Number of executions : " << taskInfo->num_of_times << " times\n";

           #if VERBOSE_INFO
           cout << "\n\nRunning summary for task" << taskInfo->name << endl;
           cout << "Average runtime : " << taskInfo->average_runtime/ 1.0e6 << " ms" <<endl;
           cout << "Max runtime : " << taskInfo->max_runtime/ 1.0e6 << " ms" << endl;
           cout << "Min runtime : " << taskInfo->min_runtime/ 1.0e6 << " ms" << endl;
           cout << "Deadline : " << taskInfo->deadline << " ms"<< endl;
           cout << "Out of Deadline : " << taskInfo->out_deadline << " times" << endl;
           cout << "Number of executions : " << taskInfo->num_of_times << " times" << endl;
           #endif

  }
  myfile.close();
  mutex.unlock();
   exit(1);
}


int main(int argc, char* argv[])
{
  system("clear");

  // Définition fichier d'information des tâches
  string input_file = "./input_chaine.txt";

  int return_code = 0;
  nproc = get_nprocs();

  cout << "Hey, press a key to start (PID: " << getpid() << ")!" << endl;
  cin.get();
  #if VERBOSE_INFO
  cout << " Generating Task Set ..." << endl;
  #endif
  buildSet bS;
  bS.readChainsList( input_file, &list_info_chaine);

  //Création de la sémaphore
  rt_sem_create(&mysync,"MySemaphore",0,S_FIFO);

  TaskLauncher tln;

  for(int i=0; i < (int)list_info_chaine.size(); ++i )
  {
      tln.readTasksList(list_info_chaine[i].Path);
  }

  //AlltasksInfosList.insert (AlltasksInfosList.end(),tln.tasksInfosList.begin(),tln.tasksInfosList.end());
   tasl=&tln;
  // cout<<"AlltasksInfosList  size :"<< AlltasksInfosList.size()<<endl;
   //tln.printTasksInfos();
   tln.runTasks();


  #if VERBOSE_INFO
    cout << "Now launching the MoCoAgent ! " << endl;
  #endif

  RT_TASK mcAgent;
  rt_task_create(&mcAgent, "MoCoAgent", 0, 2, 0);
  set_affinity(&mcAgent, 3);

  systemRTInfo ch_taks ;
  ch_taks.rtTIs=tln.tasksInfosList ;
  ch_taks.e2eDD =list_info_chaine;
  rt_task_start(&mcAgent, RunmcAgentMain, &ch_taks);


  //sleeping the time that all tasks will be started
  usleep(1000000);

  cout << "Wake up all tasks." << endl;
  rt_sem_broadcast(&mysync);

  printf("\nType CTRL-C to end this program\n\n" );

  struct sigaction sigIntHandler;

  sigIntHandler.sa_handler = my_handler;
  sigemptyset(&sigIntHandler.sa_mask);
  sigIntHandler.sa_flags = 0;

  while (1) {
    sigaction(SIGINT, &sigIntHandler, NULL);
  }

  pause();

  return return_code;
}

void printInquireInfo()
{
#if VERBOSE_INFO
  RT_TASK_INFO curtaskinfo;
  rt_task_inquire(NULL, &curtaskinfo);
  std::stringstream ss;
  ss << "I am task : " << curtaskinfo.name
     << " (PID : " << curtaskinfo.pid << "), of priority "
     << curtaskinfo.prio << endl
     << "On CPU : " << curtaskinfo.stat.cpu << endl;

  cout << ss.rdbuf();
#endif
}

void printTaskInfo(rtTaskInfosStruct* task)
{
#if VERBOSE_INFO
  std::stringstream ss;
  ss << "Name: "       << task->name
     << "| path: "     << task->path_task
     << "| is RT ? "   << task->isHardRealTime
     << "| Period: "   << task->periodicity
     << "| Deadline: " << task->deadline
     << "| affinity: " << task->affinity
     << "| ID :"       << task->ID
     << endl;
  cout << ss.rdbuf();
#endif
}

void set_affinity (RT_TASK* task, int _aff)
{
  cpu_set_t mask;
  CPU_ZERO(&mask);
  CPU_SET(_aff, &mask);
  RT_TASK_INFO curtaskinfo;
  rt_task_inquire(task, &curtaskinfo);
  cout << "Setting affinity for task " << curtaskinfo.name << " : " << rt_task_set_affinity(task, &mask) << endl;
}

void print_affinity(pid_t _pid)
{
#if VERBOSE_INFO
  int pid = _pid;
  if (!_pid)
  {
   RT_TASK_INFO curtaskinfo;
   rt_task_inquire(NULL, &curtaskinfo);
   pid = curtaskinfo.pid;
  }
  cpu_set_t mask;

  if (sched_getaffinity(_pid, sizeof(cpu_set_t), &mask) == -1) {
      perror("sched_getaffinity");
      assert(false);
  } else {
    long i;
    std::stringstream ss;
    ss << "Affinity of thread " << pid << " = ";
    for (i = 0; i < nproc; i++)
        ss << CPU_ISSET(i, &mask);
    ss << endl;
    cout << ss.rdbuf();
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

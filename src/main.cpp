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
#include "buildSet.h"
#include <mutex>


#define EXECTIME   2e8   // execution time in ns
#define SPINTIME   1e7   // spin time in ns
#define PRINT 1

long nproc;
RT_SEM mysync;
RT_BUFFER bf;
std::vector<end2endDeadlineStruct> list_info_chaine;

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
  printInquireInfo();


  MacroTask macroRT;
  macroRT.properties = *rtTI;
  macroRT.executeRun(&mysync, &bf);

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
           myfile << "Deadline :" << taskInfo->deadline / 1.0e6 << " ms\n";
           myfile << "Out of Deadline : " << taskInfo->out_deadline << " times\n";
           myfile << "Number of executions : " << taskInfo->num_of_times << " times\n";

           #if PRINT
           cout << "\n\nRunning summary for task" << taskInfo->name << endl;
           cout << "Average runtime : " << taskInfo->average_runtime/ 1.0e6 << " ms" <<endl;
           cout << "Max runtime : " << taskInfo->max_runtime/ 1.0e6 << " ms" << endl;
           cout << "Min runtime : " << taskInfo->min_runtime/ 1.0e6 << " ms" << endl;
           cout << "Deadline : " << taskInfo->deadline / 1.0e6 << " ms"<< endl;
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

  string input_file;


  // Définition fichier d'information des tâches

	input_file = "./input_chaine.txt";

  int return_code = 0;
  nproc = get_nprocs();


  cout << " Set de tâches en cours de création ..." << endl;
  buildSet bS;

  bS.readChainsList( input_file, &list_info_chaine);

  //Création de la sémaphore
  rt_sem_create(&mysync,"MySemaphore",0,S_FIFO);

  //Création du Buffer de Com entre les tasks et et McAGent
  int ret;
	ret = rt_buffer_create(&bf , "buffer_demo" , 1024 , B_FIFO);
	printf("%d\n",ret);
	if( 0 > ret)
	{
		rt_buffer_delete(&bf);
		printf("%s\n","fail creat");
	}



 for(int i=0; i < (int)list_info_chaine.size();i++ ){

    TaskLauncher tln( list_info_chaine[i].Path, list_info_chaine[i].taskChainID);
    //tln.tasksInfos = readTasksList(input_file);
    tln.printTasksInfos();
    tln.runTasks();
    tln.printTasksInfos();
    tasl=&tln;

  }
  //sleeping the time that all tasks will be started
  usleep(1000000);
  cout<<"wake up all tasks\n"<<endl;
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
     << "| path: "     << task->path
     << "| is RT ? "   << task->isHardRealTime
     << "| Period: "   << task->periodicity
     << "| Deadline: " << task->deadline
     << "| affinity: " << task->affinity << endl;
  cout << ss.rdbuf();
#endif
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

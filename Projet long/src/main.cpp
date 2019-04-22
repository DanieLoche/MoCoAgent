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
//#include "buildSet.h"
#include <mutex>

#define EXECTIME   2e8   // execution time in ns
#define SPINTIME   1e7   // spin time in ns

long nproc;
RT_SEM mysync;

TaskLauncher* tasl;
MCAgent* mca;
string inputFile = "input_chaine.txt", outputFile = "ExpeOutput.csv2";

void RunmcAgentMain(void* arg)
{
  systemRTInfo* sInfos = (systemRTInfo*) arg;
  //printTaskInfo(&sInfos->rtTIs[0]);
  mca = new MCAgent(sInfos);
}


void TaskMain(void* arg)
{
  taskRTInfo* _taskRTInfo = (taskRTInfo*) arg;
  MacroTask macroRT(_taskRTInfo);
  rt_sem_p(&mysync,TM_INFINITE);
  if (_taskRTInfo->rtTI->isHardRealTime == 0) {
    macroRT.executeRun_besteffort();
  }
  else {
    macroRT.executeRun();
  }
}


void endOfExpeHandler(int s){
  #if VERBOSE_INFO
  cout << "\n------------------------------" << endl;
  #endif
  tasl->saveData(outputFile);
  sleep (3);
  mca->saveData("MCAgent_"+outputFile);

   exit(1);
}


int main(int argc, char* argv[])
{
    system("clear");
    int return_code = 0;
    nproc = get_nprocs();

    struct sigaction sigIntHandler;

    sigIntHandler.sa_handler = endOfExpeHandler;
    sigIntHandler.sa_flags = 0;
    sigemptyset(&sigIntHandler.sa_mask);
    sigaction(SIGINT, &sigIntHandler, NULL);


    bool enableAgent = TRUE;
    long expeDuration = 0;
    // Définition fichier d'information des tâches
    // [MoCoAgent Activation] [Experiment duration] [input file : task chains] [outputfile]
    if (argc > 1)
    { // setting enableAgent boolean
      std::stringstream ss(argv[1]);
      if(!(ss >> std::boolalpha >> enableAgent)) {
        printf("Error, %s is not an bool", argv[1]);
        return EXIT_FAILURE;
      }
      if (argc > 2)
      { // setting expeDuration in seconds
        if(sscanf(argv[2], "%ld", &expeDuration) != 1) {
          printf("Error, %s is not an int", argv[2]);
          return EXIT_FAILURE;
        }
        if (argc > 3)
        {
          inputFile = argv[3];
          if (argc > 4)
          {
            outputFile = argv[4];
          }
        }
      }
    }
    else {
      cout << "Missing arguments. Format should be : " << endl
          << "[MoCoAgent Activation] [Experiment duration]"
          << "[input file : task chains]" << endl;
          cout << "if set to \"-\", default values are : " << endl
          << enableAgent << "  |   " << "no end time" << "   |   " << inputFile << "   |   " << endl;
          return 0;
        }

    cout << "Hey, press a key to start (PID: " << getpid() << ")!" << endl;
    cin.get();

    rt_sem_create(&mysync,"Start Experiment",0,S_FIFO);

    TaskLauncher tln;

    #if VERBOSE_INFO
    cout << " Generating Task Set ..." << endl;
    #endif
    tln.readChainsList(inputFile);
    tln.readTasksList();

    tln.printTasksInfos();

    tln.runTasks();

    if (enableAgent)
    {
      sleep(1);
      tln.runAgent();
    }

    //sleeping the time that all tasks will be started
    sleep(2);
    cout << "Wake up all tasks." << endl;
    rt_sem_broadcast(&mysync);

    printf("\nType Ctrl + C to end this program\n\n" );
//    string ss;
//    while (ss != "STOP") cin >> ss;

    tasl=&tln;

    //while (1) {    }
    if (expeDuration) sleep(expeDuration);
    else pause();
    endOfExpeHandler(0);

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
#if VERBOSE_DEBUG
  std::stringstream ss;
  ss << "Name: "       << task->name
     << "| path: "     << task->path_task
     << "| is RT ? "   << task->isHardRealTime
     << "| Period: "   << task->periodicity
     << "| Deadline: " << task->deadline
     << "| affinity: " << task->affinity
     << "| ID :"       << task->id
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
  if (rt_task_set_affinity(task, &mask))
  {
    cout << "Error while setting affinity for task " << curtaskinfo.name << endl;
  }
    #if VERBOSE_ASK
    cout << "Setting affinity for task " << curtaskinfo.name << " : " << _aff << endl;
    #endif
}

void print_affinity(pid_t _pid)
{
#if VERBOSE_ASK
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

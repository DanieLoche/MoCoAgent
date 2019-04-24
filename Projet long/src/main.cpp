// #define VERBOSE_INFO  1 // Cout d'informations, démarrage, etc...
// #define VERBOSE_DEBUG 0 // Cout de débug...
// #define VERBOSE_OTHER 1 // Cout autre...
// #define VERBOSE_ASK   1 // cout explicitement demandés dans le code

#include "tools.h"
#include "sched.h"

#include "mcAgent.h"
#include "macroTask.h"
#include "taskLauncher.h"
//#include "buildSet.h"
//#include <mutex>
#include <sys/sysinfo.h>
#include <iomanip>


#define EXECTIME   2e8   // execution time in ns
#define SPINTIME   1e7   // spin time in ns

long nproc;
RT_SEM mysync;

TaskLauncher* tln;
MCAgent* mca;
bool enableAgent = TRUE;
long expeDuration = 0;
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
  cout << "Changing mode to OFF." << endl;
  mca->setMode(0); // Stop Mo Co Agent verification
  cout << "Stopping Tasks." << endl;
  tln->stopTasks(1);
  cout << "Saving tasks data..." << endl;
  tln->saveData(outputFile);
  if (enableAgent)
  {
     sleep (3);
     cout << "Saving Agent data..." << endl;
     mca->saveData("MCAgent_"+outputFile);
  }
   exit(0);
}


int main(int argc, char* argv[])
{
   system("clear");
   nproc = get_nprocs();

   struct sigaction sigIntHandler;
   sigIntHandler.sa_handler = endOfExpeHandler;
   sigIntHandler.sa_flags = 0;
   sigemptyset(&sigIntHandler.sa_mask);
   sigaction(SIGINT, &sigIntHandler, NULL);

   tln = new TaskLauncher();

   // Définition fichier d'information des tâches
   // [MoCoAgent Activation] [Experiment duration] [input file : task chains] [outputfile]
   //// "--" to set everything as default.
   if (argc > 1)
   {
      std::stringstream ss(argv[1]);
      if (ss.str() == "-h" || ss.str() == "help" || ss.str() == "--help")
      { // HELP message
         cout  << "Format should be : " << endl
               << argv[0] << " [MoCoAgent Activation] " << " | " << " [duration] " << " | "
               << std::setw(inputFile.length()) << "[input file]" << " | "
               << std::setw(outputFile.length()) << "[output file]" << '\n';
         cout  << std::boolalpha << "Default using \"-\" : "
               << std::setw(11) << enableAgent << std::setw(11) << " | " << "No end time." << " | "
               << std::setw(inputFile.length()) << inputFile << " | "
               << std::setw(outputFile.length()) << outputFile << endl;
         return 0;
      }
      else if(ss.str() != "-" && !(ss >> std::boolalpha >> enableAgent))
      { // ENABLE MoCoAgent - parse true/false as bool
         printf("Error, %s is not an bool", argv[1]); return EXIT_FAILURE;
      }

      if (argc > 2)
      { // EXPE DURATION (s)
         std::stringstream ss(argv[2]);
         if(sscanf(argv[2], "%ld", &expeDuration) != 1 && ss.str() != "-")
         { printf("Error, %s is not an int", argv[2]); return EXIT_FAILURE; }

         if (argc > 3)
         { // INPUT FILE
            std::stringstream ss(argv[3]);
            if (ss.str() != "-") inputFile = argv[3];

            if (argc > 4)
            { // OUTPUT FILE
               std::stringstream ss(argv[4]);
               if (ss.str() != "-") outputFile = argv[4];

               if (argc > 5)
               { // SCHEDULING POLICY
                  std::stringstream ss(argv[5]);
                  string _schedMode = ss.str();
                  if (_schedMode == "FIFO")     tln->schedPolicy = SCHED_FIFO;
                  else if  (_schedMode == "RM") tln->schedPolicy = SCHED_RM;
                  else if  (_schedMode == "RR") tln->schedPolicy = SCHED_RR;
                  else if  (_schedMode == "EDF") tln->schedPolicy = SCHED_RM;
               }
            }
         }
      }
   }

   cout << "Hey, press a key to start (PID: " << getpid() << ")!" << endl;
   cin.get();

   rt_sem_create(&mysync,"Start Experiment",0,S_FIFO);

   #if VERBOSE_INFO
   cout << " Generating Task Set ..." << endl;
   #endif
   tln->readChainsList(inputFile);
   tln->readTasksList();

   tln->printTasksInfos();

   tln->createTasks();
   tln->runTasks();

   if (enableAgent)
   {
      sleep(1);
      tln->runAgent();
   }

   //sleeping the time that all tasks will be started
   sleep(2);
   cout << "Wake up all tasks." << endl;
   rt_sem_broadcast(&mysync);

   printf("\nType Ctrl + C to end this program\n\n" );
   //    string ss;
   //    while (ss != "STOP") cin >> ss;

   //while (1) {    }
   if (expeDuration) sleep(expeDuration);
   else pause();
   cout << "End of Experimentation." << endl;
   endOfExpeHandler(0);

   return 0;
}

void printInquireInfo(RT_TASK* task)
{
#if VERBOSE_INFO
   RT_TASK_INFO curtaskinfo;
   int ret = 0;
   if (task == NULL) cout << "Task in NULL !! F**k !" << endl; //task = rt_task_self();
   if ((ret = rt_task_inquire(task, &curtaskinfo)))
   {  cout << ret;
      if (ret == -EINVAL) cout << " - Invalid Task Descriptor or invalid Prio." << endl;
      if (ret == -EPERM) cout << " - Task is NULL, and service called from invalid context." << endl;
   } else {
   std::stringstream ss;
   ss << "I am task : " << curtaskinfo.name
     << " (PID : " << curtaskinfo.pid << "), of priority "
     << curtaskinfo.prio
     << " - on CPU : " << curtaskinfo.stat.cpu << endl;
   cout << ss.rdbuf();
   }
#endif
}

void printTaskInfo(rtTaskInfosStruct* task)
{
#if VERBOSE_OTHER
  std::stringstream ss;
  ss << "Name: "       << task->name
     << "| path: "     << task->path_task
     << "| is RT ? "   << task->isHardRealTime
     << "| Deadline: " << task->deadline
     << "| affinity: " << task->affinity
     << "| ID :"       << task->id
     << endl;
  cout << ss.rdbuf();
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

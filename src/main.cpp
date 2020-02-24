// #define VERBOSE_INFO  1 // Cout d'informations, démarrage, etc...
// #define VERBOSE_DEBUG 0 // Cout de débug...
// #define VERBOSE_OTHER 1 // Cout autre...
// #define VERBOSE_ASK   1 // cout explicitement demandés dans le code

#include <sys/sysinfo.h>
#include <iomanip>

#include "taskLauncher.h"
#include "NanoLog.h"
#include "tools.h"

long nproc;

TaskLauncher* tln;
//MCAgent* mca;

string inputFile = "input_chaine.txt", outputFile = "ExpeOutput";
int enableAgent = 1; // 0 = Disable | 1 = Enable | 2 = Enable for monitoring only
int expeDuration = 60;
int schedMode = SCHED_FIFO, cpuFactor = 100;

static void show_usage(bool status)
{
   cout  << "Format should be : " << '\n' << program_invocation_name
         << " [-h | --help] | [-i input] [-o output] [-d duration] [-l load] [-e enable] [[]-s schedPolicy] | [-RR | -FIFO | -RM | -EDF]] " << '\n';
   cout  << std::setw(sizeof(program_invocation_name)) << "Default is : "
         << inputFile << " | " << outputFile << " | " << "No end time" << " | " << " 100% " << " | " << enableAgent << " | " << schedMode << " | " << endl;
   exit(status);
}

int main(int argc, char* argv[])
{
    nproc = get_nprocs();

    //int i = ERROR_MNG(atexit(endOfExpeHandler));

    for( int i = 1; i < argc; ++i)
    {
      std::string arg = argv[i];
      cout << "Doing argument #" << i << " = " << arg << "." << endl;
      if (arg == "-h" || arg == "--help" || arg == "help")
         show_usage(EXIT_SUCCESS);
      else if (arg == "-i" || arg == "--input")
      {
         if (i+1 < argc) inputFile = argv[++i];
         else {printf("Error : argument missing after option for Input file name.\n");
               show_usage(EXIT_FAILURE); }
      }
      else if (arg == "-o" || arg == "--output")
      {
         if (i+1 < argc) outputFile = argv[++i];
         else {printf("Error : argument missing after option for Output file name.\n");
               show_usage(EXIT_FAILURE); }
      }
      else if (arg == "-d" || arg == "--duration")
      { // EXPE DURATION (s)
         TRY_CONV("Expe. duration", argv, expeDuration);
      }

      else if (arg == "-l" || arg == "--load")
      { // CPU % Factor
         TRY_CONV("cpu factor", argv, cpuFactor);
      }

      else if (arg == "-e" || arg == "--enable")
      {// ENABLE MoCoAgent - parse true/false as bool
         TRY_CONV("MoCoAgent mode", argv, enableAgent);
      }

      else if (arg == "-s" || arg == "--schedPolicy")
      { // Sched policy by value.
         TRY_CONV("scheduling policy", argv, schedMode);
      }
      else if (arg == "-FIFO")   schedMode = SCHED_FIFO;
      else if  (arg == "-RM")    schedMode = SCHED_RM;
      else if  (arg == "-RR")    schedMode = SCHED_RR;
      else if  (arg == "-EDF")   schedMode = SCHED_EDF; // NOT MANAGED !!

    }

    cout << "Experiment made with parameters : \n"
      << " MoCoAgent: " << enableAgent  << "\n"
      << "  Duration: " << expeDuration << "\n"
      << "CPU Factor: " << cpuFactor    << "\n"
      << "Input  file: " << inputFile   << "\n"
      << "Output files: " << outputFile << "_Expe.csv & "
      << outputFile       << "_Chains.csv"  << " & "
      << outputFile       << "_Resume.txt"    << endl;

   std::ofstream outputFileResume;
   string outputFileName = outputFile + "_Resume.txt";
   outputFileResume.open (outputFileName, std::ios::app);    // TO APPEND :  //,ios_base::app);
   outputFileResume << "Experiment made with parameters : \n"
      << " MoCoAgent: " << enableAgent  << "\n"
      << "  Duration: " << expeDuration << "\n"
      << "CPU Factor: " << cpuFactor    << "\n"
      << "Input  file: " << inputFile   << "\n"
      << "Output files: " << outputFile << "_Expe.csv & "
      << outputFile       << "_Chains.csv"  << " & "
      << outputFile       << "_Resume.txt"    << endl;

   outputFileResume.close();

   //cout << "Press a key to start (PID: " << getpid() << ")!" << endl;
   //cin.get();

   tln = new TaskLauncher(outputFile, schedMode);

   #if VERBOSE_INFO
   cout << "\n------------------------------" << endl;
   cout << " Generating Task Set ..." << endl;
   #endif
   if(tln->readChainsList(inputFile)) {cerr << "Failed to read task chains." << endl; return -1;}
   if(tln->readTasksList(cpuFactor)) {cerr << "Failed to read tasks list." << endl; return -2;}
   //   if(tln->createMutexes(nproc)) {cout << "Failed to read tasks list." << endl; return -3;}

   tln->printTasksInfos();

   if(tln->runTasks(expeDuration)) {cerr << "Failed to create all tasks" << endl; return -4;}
   if (enableAgent)
   {
      sleep(2);
      tln->runAgent(expeDuration);
   }

   //sleeping the time that all tasks will be started
   sleep(2);
   //cout << "Wake up all tasks." << endl;
   /*
      int bak_fd = dup(1);
      fflush(stdout);
      int new_fd = open("/dev/null", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
      dup2(new_fd, 1); //fflush(stdout);
   */


   //cinbuf = std::cin.rdbuf(); //save stdIn
   //coutbuf = std::cout.rdbuf(); //save stdOut

   //    string ss;
   //    while (ss != "STOP") cin >> ss;

   //if (expeDuration) sleep(expeDuration);
   //else pause();

   //sleep(2);
   //fflush(stdout);
   /*
      dup2(bak_fd, 1);
      close(bak_fd);
      fflush(stdout);
   */
   //std::cin.rdbuf(cinbuf);   //reset to standard input again
   //std::cout.rdbuf(coutbuf); //reset to standard output again
   cout << "End of Experimentation." << endl;

   exit(EXIT_SUCCESS);
}

void printInquireInfo(RT_TASK* task)
{
   #if VERBOSE_INFO
   RT_TASK_INFO curtaskinfo;
   int ret = 0;
   if ((ret = rt_task_inquire(task, &curtaskinfo)))
   {  cout << "\n Inquire Error (" << ret;
      if (ret == -EINVAL) cerr << ") - Invalid Task Descriptor or invalid Prio." << endl;
      if (ret == -EPERM) cerr << ") - Task is NULL, and service called from invalid context." << endl;
   } else {
      rt_printf("[ %s ] - (PID : %d) - Priority = %d.\n", curtaskinfo.name, curtaskinfo.pid, curtaskinfo.prio);
   }
   #endif
}

void printTaskInfo(rtTaskInfosStruct* task)
{
   #if VERBOSE_OTHER
   std::stringstream ss;
   ss << "___ ["       << task->fP.name << "] - parameter summary : "
      << "\n" << "  |- ID :"       << task->fP.id << " (isHRT : " << task->fP.isHRT << ")"
      << "\n" << "  |- func : "     << task->fP.func << " | Args : " << task->fP.args
      << "\n" << "  |- Period: "   << task->rtP.periodicity
              << " | on core : " << task->rtP.affinity
              << " | at priority : " << task->rtP.priority << " | Policy: "   << getSchedPolicyName(task->rtP.schedPolicy) << "(" << task->rtP.schedPolicy << ")"
      << "\n" << "  |- WCET = " << task->fP.wcet << " | Precedent task ID : " << task->fP.prec
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

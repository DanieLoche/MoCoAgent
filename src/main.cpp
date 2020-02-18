// #define VERBOSE_INFO  1 // Cout d'informations, démarrage, etc...
// #define VERBOSE_DEBUG 0 // Cout de débug...
// #define VERBOSE_OTHER 1 // Cout autre...
// #define VERBOSE_ASK   1 // cout explicitement demandés dans le code

#include <sys/sysinfo.h>
#include <iomanip>

#include "taskLauncher.h"
#include "tools.h"

long nproc;

TaskLauncher* tln;
//MCAgent* mca;
int enableAgent = 1; // 0 = Disable | 1 = Enable | 2 = Enable for monitoring only
long expeDuration = 0;
string inputFile = "input_chaine.txt", outputFile = "ExpeOutput";
int schedMode = SCHED_FIFO, cpuFactor = 100;

/*
std::streambuf *cinbuf;
std::streambuf *coutbuf;
bool HandleOnce = false;
void endOfExpeHandler(void){
   if (!HandleOnce)
   {
      HandleOnce = true;
      tln->saveData(outputFile);
      #if VERBOSE_INFO
      cout << "\n------------------------------" << endl;
      #endif
   }
   sleep(2);

}
*/

int main(int argc, char* argv[])
{
    nproc = get_nprocs();

    //int i = ERROR_MNG(atexit(endOfExpeHandler));

    // Définition fichier d'information des tâches
    // [MoCoAgent Activation] [Experiment duration] [cpuFactor%] [input file : task chains] [outputfile] [sched policy]
    //// "--" to set everything as default.
    if (argc > 1)
    { std::stringstream ss(argv[1]); // HELP or Agent Enable
        if (ss.str() == "-h" || ss.str() == "help" || ss.str() == "--help")
        { // HELP message
            cout  << "Format should be : " << endl
                << argv[0] << " [MoCoAgent Activation] " << " | " << " [duration] " << " | " << "[wcet %]" << " | "
                << std::setw(inputFile.length()) << "[input file]" << " | "
                << std::setw(outputFile.length()) << "[output file]" << '\n';
            cout  << std::boolalpha << "Default using \"-\" : "
                << std::setw(11) << enableAgent << std::setw(11) << " | " << "No end time." << " | " << " 100 %  "
                << std::setw(inputFile.length()) << inputFile << " | "
                << std::setw(outputFile.length()) << outputFile << endl;
            return 0;
        }
        else if(ss.str() != "-" && sscanf(argv[1], "%d", &enableAgent) != 1)
        { // ENABLE MoCoAgent - parse true/false as bool
            printf("Error, %s is not an int ?!", argv[1]); return EXIT_FAILURE;
        }
    if (argc > 2)
    { std::stringstream ss(argv[2]); // EXPE DURATION (s)
        if(ss.str() != "-" && sscanf(argv[2], "%ld", &expeDuration) != 1)
            { printf("Error, %s is not an int", argv[2]); return EXIT_FAILURE; }

    if (argc > 3)
    { std::stringstream ss(argv[3]);  // CPU % Factor
        if(ss.str() != "-" && sscanf(argv[3], "%d", &cpuFactor) != 1)
            { printf("Error, %s is not an int", argv[3]); return EXIT_FAILURE; }

    if (argc > 4)
    { std::stringstream ss(argv[4]);    // INPUT FILE
        if (ss.str() != "-") inputFile = argv[4];

    if (argc > 5)
    { std::stringstream ss(argv[5]);     // OUTPUT FILE
        if (ss.str() != "-") outputFile = argv[5];

    if (argc > 6)
    { std::stringstream ss(argv[6]);    // SCHEDULING POLICY
        string _schedMode = ss.str();
        if (_schedMode == "FIFO")     schedMode = SCHED_FIFO;
        else if  (_schedMode == "RM") schedMode = SCHED_RM;
        else if  (_schedMode == "RR") schedMode = SCHED_RR;
        else if  (_schedMode == "EDF") schedMode = SCHED_RM;
    } // if arg 6
    } // if arg 5
    } // if arg 4
    } // if arg 3
    } // if arg 2
    } // if arg 1

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
   {  cout << ret;
      if (ret == -EINVAL) cerr << "\n - Invalid Task Descriptor or invalid Prio." << endl;
      if (ret == -EPERM) cerr << "\n - Task is NULL, and service called from invalid context." << endl;
   } else {
      std::stringstream ss;
      ss << "[ " << curtaskinfo.name
      << " ] (PID : " << curtaskinfo.pid << "), of priority "
      << curtaskinfo.prio << endl;
      cout << ss.rdbuf();
   }
   #endif
}

void printTaskInfo(rtTaskInfosStruct* task)
{
   #if VERBOSE_OTHER
   std::stringstream ss;
   ss << "Name: "       << task->fP.name
   << "| func: "     << task->fP.func
   << "| is RT ? "   << task->fP.isHRT
   << "| Deadline: " << task->rtP.periodicity
   << "| affinity: " << task->rtP.affinity
   << "| priority: " << task->rtP.priority
   << "| schedPolicy: " << task->rtP.schedPolicy
   << "| ID :"       << task->fP.id
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

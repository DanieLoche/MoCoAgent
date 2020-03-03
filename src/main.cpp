#include <iomanip>

#include "taskLauncher.h"
#include "NanoLog.h"
#include "tools.h"

//MCAgent* mca;

string inputFile = "input_chaine.txt", outputFile = "RES_60_1_FIFO_100";
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
    //nproc = get_nprocs();
    //setvbuf(stderr, NULL, _IOLBF, 4096) ;
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
      << " MoCoAgent mode: " << enableAgent  << "\n"
      << "  Duration: " << expeDuration << "\n"
      << "CPU Factor: " << cpuFactor    << "\n"
      << "Scheduling Policy: " << getSchedPolicyName(schedMode) << "("<< schedMode <<")" << "\n"
      << "Input  file: " << inputFile   << "\n"
      << "Output files: "  << outputFile << RESUME_FILE << " & "
                           << outputFile << CHAIN_FILE  << " & "
                           << outputFile << TASKS_FILE  << endl;

   std::ofstream outputFileResume;
   string outputFileName = outputFile + RESUME_FILE;
   outputFileResume.open (outputFileName);    // TO APPEND :  //,ios_base::app);
   outputFileResume << "Experiment made with parameters : \n"
      << " MoCoAgent: " << enableAgent  << "\n"
      << "  Duration: " << expeDuration << "\n"
      << "CPU Factor: " << cpuFactor    << "\n"
      << "Scheduling Policy: " << getSchedPolicyName(schedMode) << "("<< schedMode <<")" << "\n"
      << "Input  file: " << inputFile   << "\n"
      << "Output files: " << outputFile << "_Expe.csv & "
      << outputFile       << "_Chains.csv"  << " & "
      << outputFile       << "_Resume.txt"    << endl;

   outputFileResume.close();

   //cout << "Press a key to start (PID: " << getpid() << ")!" << endl;
   //cin.get();

   TaskLauncher* tln = new TaskLauncher(enableAgent, outputFile, schedMode);

   #if VERBOSE_INFO
   cout << "\n------------------------------" << endl;
   cout << " Generating Task Set ..." << endl;
   #endif
   if(tln->readChainsList(inputFile)) {cerr << "Failed to read task chains." << endl; return -1;}
   if(tln->readTasksList(cpuFactor)) {cerr << "Failed to read tasks list." << endl; return -2;}

   if(tln->runTasks(expeDuration)) {cerr << "Failed to create all tasks" << endl; return -4;}

   if (enableAgent)
   {
      sleep(2);
      tln->runAgent(expeDuration);
   }

   //sleeping the time that all tasks will be started
   //sleep(2);
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
   cout << " ======= END OF EXPERIMENTATION ======" << endl;
   exit(EXIT_SUCCESS);
}

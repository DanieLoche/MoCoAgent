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
        cout << "Affinity of thread " << curtaskinfo.pid << " = ";
        for (i = 0; i < nproc; i++)
            cout << CPU_ISSET(i, &mask);
        cout << endl;
        /* using printf
        printf("sched_getaffinity = ");
        for (i = 0; i < nproc; i++) {
            printf("%d ", CPU_ISSET(i, &mask));
        }
        printf("\n");
        */
    }

}

void RunmcAgentMain(void* arg)
{
  cout << "MoCoAgent running ?!" << endl;
  MCAgent mcAgent(arg);
}


void TaskMain(void* arg)
{
  rtTaskInfosStruct* rtTI = (rtTaskInfosStruct*) arg;

  RT_TASK_INFO curtaskinfo;
  rt_task_inquire(NULL, &curtaskinfo);

  //cout << "I am task : " << curtaskinfo.name << " of priority " << curtaskinfo.prio << endl;

  MacroTask macroRT;
  macroRT.properties = rtTI;
  macroRT.executeRun(&mysync);

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
  string task_file;
  string asked;
  int n_short = 0;
  int n_long = 0;
  int n_crit = 0;
  int Build = 0;
  int conf = 0;
  int ordo = 0;


  // Définition fichier d'information des tâches
  task_file = "./sorted.txt";
	input_file = "./input.txt";
/*
  cout << "Y a-t-il déjà un fichier input que vous voulez utiliser ? [Y/N]" << '\n';
  cin >> asked;
  if ((asked == "Y") | (asked == "y") | (asked == "yes") | (asked == "YES") | (asked == "Yes")){
    cout << "Quel est le nom du fichier d'entrée (avec l'extention .txt) ?" << '\n';
    cin >> input_file;
  }
  else if ((asked == "N") | (asked == "n") | (asked == "no") | (asked == "NO") | (asked == "No")) {
    Build = 1;
    input_file = "./input.txt";
    while(conf!=1){
    cout << "Combien de tâches courtes voulez-vous ?" << '\n';
    cin >> n_short;
    cout << "Combien de tâches longues voulez-vous ?" << '\n';
    cin >> n_long;
    cout << "Quel pourcentage de tâches critique désirez vous ? (en \%)" << '\n';
    cin >> n_crit;
    cout << "Vous désirez :\n" << n_short << " tâches courtes, " << n_long << " tâches longues et " << n_crit << " \% de tâches critiques. Vous confirmez ? [Y/N]\n";
    cin >> asked;
    if ((asked == "Y") | (asked == "y") | (asked == "yes") | (asked == "YES") | (asked == "Yes")){ conf = 1;}
    else if ((asked == "N") | (asked == "n") | (asked == "no") | (asked == "NO") | (asked == "No")) {conf = 0;}
    }
  }
  else {
    cout << "Pas de ça chez moi c'est Y ou N batard" << '\n';
    return 3; // 3 == mauvaise réponse du fichier en entrée
  }
  cout << "Quel Ordonnancement désirez-vous appliquer ?\n" << "1 - Round Robin (RR)\n2 - FIFO\n3 - EDF\n\n";
  cin >> ordo;
  switch (ordo) {
    case 1:
      cout << "Round Robin sera utilisé" << endl;
      break;
    case 2:
      cout << "FIFO sera utilisé" << endl;
      break;
    case 3:
      cout << "EDF sera utilisé" << endl;
      break;
    default:
      cout << "Saisie invalide, FIFO sera utilisé" << endl;
      break;
  }
  sleep(2);
  //rt_task_set_mode(0,XNRRB,NULL);
*/
  int return_code = 0;
  nproc = get_nprocs();


  if(Build == 1){
    cout << " Set de tâches en cours de création ..." << endl;
    buildSet bS;

    bS.readChainsList( input_file, &list_info_chaine);

    // Définition des listes comportant les tâches longue et courte
    std::vector<string> all_crit_tasks = bS.distributionCrit(n_long, n_short, n_crit);

    // Définition des tâches non critiques choisies
    std::vector<string> uncrit_tasks = bS.get_uncrit_tasks();

  // Edition du fichier input.txt
  //bS.buildInput();

  // Récupération infos tâches
  std::vector<rtTaskInfosStruct> info_task = bS.get_infos_tasks(task_file);

  // Edition du fichier input.txt
  bS.buildInput();
  cout << " Set de créé" << endl;
}

rt_sem_create(&mysync,"MySemaphore",0,S_FIFO);


 for(int i=0; i < (int)list_info_chaine.size();i++ ){

    TaskLauncher tln( list_info_chaine[i].Path);
    //tln.tasksInfos = readTasksList(input_file);
    tln.printTasksInfos();
    tln.runTasks();
    tasl=&tln;

  }
  //sleeping the time that all tasks will be started
  usleep(1000000);
  cout<<"wake up all tasks\n"<<endl;
  rt_sem_broadcast(&mysync);

  printf("\nType CTRL-C to end this program\n\n" );

  struct sigaction sigIntHandler;

  sigIntHandler.sa_handler = my_handler;;
  sigemptyset(&sigIntHandler.sa_mask);
  sigIntHandler.sa_flags = 0;

  while (1) {
    sigaction(SIGINT, &sigIntHandler, NULL);
  }


  pause();

  return return_code;
}

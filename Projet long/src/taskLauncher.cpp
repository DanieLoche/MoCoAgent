#include "tools.h"
#include "taskLauncher.h"
#include "sched.h"
#include "edf.h"

#include <iomanip>


TaskLauncher::TaskLauncher(string outputFileName)
{
   enableAgent = 0;

   bool trigA = new bool();
   trigA = 0;
   triggerSave = trigA;
   taskSetInfos.triggerSave = &triggerSave;
   taskSetInfos.outputFileName = outputFileName;
}


int TaskLauncher::readChainsList(string input_file)
{
   cout << "Initialising machine...\n";

   std::ifstream myFile(input_file);
   if (!myFile.is_open())
   {
      exit(EXIT_FAILURE);
   }

   string str;
   std::getline(myFile, str); // skip the first line
   while (std::getline(myFile, str))
   {
      end2endDeadlineStruct chaineInfo;
      std::istringstream iss(str);
      #if VERBOSE_ASK
      cout << "Managing line : " << str << endl;
      #endif
      if (str.substr(0,2) != "//")
      {
         if (!(iss >> chaineInfo.name
                   >> chaineInfo.taskChainID
                   >> chaineInfo.Path
                   >> chaineInfo.deadline ))
            { cerr << "Failed to read line : " << str << endl; return -1; } // error
         chaineInfo.deadline *= 1.0e6;
         taskSetInfos.e2eDD.push_back(chaineInfo);
      }
      #if VERBOSE_ASK
      else cout << "line ignored." << endl;
      #endif
   }
   return 0;
}

int TaskLauncher::readTasksList(int cpuPercent)
{
   float cpuFactor = cpuPercent/100.0;
   cout << "CPU FACTOR IS : " << cpuFactor << endl;
   for(int i=0; i < (int)taskSetInfos.e2eDD.size(); ++i )
   {
      std::ifstream myFile(taskSetInfos.e2eDD[i].Path);
      if (!myFile.is_open())
      {
         cerr << "Failed to open file : " << taskSetInfos.e2eDD[i].Path << endl;
         exit(EXIT_FAILURE);
      }

      string str;
      std::getline(myFile, str); // skip the first line
      while (std::getline(myFile, str))
      {
         rtTaskInfosStruct* taskInfo = new rtTaskInfosStruct;
         std::istringstream iss(str);
         string token;
         char ext[32] = "RT_";
         #if VERBOSE_ASK
         cout << "Managing line : " << str << endl;
         #endif
         if (str.substr(0,2) != "//")
         {
            float tmp_wcet = 0;
            char name[28];
            if (!(iss >> taskInfo->id >> name
                      >> tmp_wcet     // WCET -> for task chain // placeholder.
                      >> taskInfo->periodicity               // meanET -> period
                      >> taskInfo->isHardRealTime
                      >> taskInfo->affinity
                      >> taskInfo->precedency
                      >> taskInfo->path_task ) )
               { cerr << "Failed to read line : " << str << endl; return -1; } // error
            //taskInfo->isHardRealTime = taskSetInfos.e2eDD[i].taskChainID;
            getline(iss , taskInfo->arguments);
            taskInfo->arguments = reduce(taskInfo->arguments);
            taskInfo->priority = 50;
            // Traitement de la périodicité de la tâche
            taskInfo->wcet = tmp_wcet * 1.0e6;              // conversion ms to RTIME (ns)
            taskInfo->periodicity = taskInfo->periodicity * 1.0e6 * cpuFactor; //taskInfo->periodicity = taskInfo->periodicity * 1.0e6 * cpuFactor;
            // Traitement du nom de la tâche
            strncat(ext, name, 64);
            strcpy(taskInfo->name,ext);
            //printTaskInfo(&taskInfo); // Résumé

            taskSetInfos.rtTIs.push_back(*taskInfo);
         }
         #if VERBOSE_ASK
         else cout << "line ignored." << endl;
         #endif
      }

      if (schedPolicy == SCHED_RM)
      { // Changer les niveaux de priorité si on schedule en RM.
         cout << "Updating task informations to use Rate-Monotinic Scheduling." << endl;
         std::sort(taskSetInfos.rtTIs.begin(), taskSetInfos.rtTIs.end(), sortAscendingPeriod());
         int prio = 10;
         RTIME lastPeriod = taskSetInfos.rtTIs[0].periodicity;
         for (auto taskInfo = taskSetInfos.rtTIs.begin(); taskInfo != taskSetInfos.rtTIs.end(); ++taskInfo)
         {
            if (taskInfo->periodicity != lastPeriod)
            {
               prio += 2;
               lastPeriod = taskInfo->periodicity;
            }
            taskInfo->priority = prio;
         }
         schedPolicy = SCHED_FIFO;
      }
   }
   return 0;
}

/*
int TaskLauncher::createMutexes(int _nprocs)
{
   cout << "Creating mutexes." << endl;
   int ret = 0;
   string mutexName = "mutCore";
   for (int i = 0; i < _nprocs; i++)
   {
      string name = mutexName + std::to_string(i);
      ret += rt_mutex_create(&mutexes[i], &name[0]);
      cout << "Created mutex " << name << endl;
   }

   return ret;
}
*/

int TaskLauncher::createTasks()
{
   int ret = 0;
   #if VERBOSE_INFO
   cout << endl << "CREATING TASKS : " << endl;
   #endif
   //for (auto taskInfo = taskSetInfos.rtTIs.begin(); taskInfo != taskSetInfos.rtTIs.end(); ++taskInfo)
   for (auto& taskInfo : taskSetInfos.rtTIs)
   {
      taskInfo.task = new RT_TASK;
      #if VERBOSE_INFO
      cout << "Creating Task " << taskInfo.name << "." << endl;
      #endif

      TaskDataLogger* dlog = new TaskDataLogger(&taskInfo);
      taskRTInfo* _taskRTInfo = new taskRTInfo;
      _taskRTInfo->taskLog = dlog;
      _taskRTInfo->rtTI = &taskInfo;
      //printInquireInfo(taskInfo.task);
      int local_errno;
      pthread_attr_t attr;
      if ( (local_errno = pthread_attr_init(&attr)) != 0)
      {
         cerr << "[" << taskInfo.name << "] " << "Failed to setup tasks parameters." << endl;
      }
      else
      {
         struct sched_param sp;
         local_errno = pthread_attr_getschedparam(&attr, &sp);
         if (local_errno != 0) { cerr << "[" << taskInfo.name << "] " << "Failed to get tasks parameters. (err " << local_errno << ")" << endl; }
         local_errno = pthread_attr_setinheritsched(&attr, PTHREAD_INHERIT_SCHED);
         if (local_errno != 0) { cerr << "[" << taskInfo.name << "] " << "Failed to set scheduling inheritance. (err " << local_errno << ")" << endl; }
         local_errno = pthread_attr_setschedpolicy(&attr, schedPolicy);
         if (local_errno != 0) { cerr << "[" << taskInfo.name << "] " << "Failed to set scheduling policy. (err " << local_errno << ")" << endl; }
         sp.sched_priority = taskInfo.priority;
         local_errno = pthread_attr_setschedparam(&attr, &sp);
         if (local_errno != 0) { cerr << "[" << taskInfo.name << "] " << "Failed to set tasks parameters. (err " << local_errno << ")" << endl; }
      }



      if(pthread_create(taskInfo.task, &attr, TaskMain, _taskRTInfo) < 0)
      {
         cerr << "[" << taskInfo.name << "] " << "Failed to create task. (err " << local_errno << ")" << endl;
         return -1;
      }
      else
      {
         RT_TASK_INFO curtaskinfo;
         rt_task_inquire(taskInfo.task, &curtaskinfo);
         struct sched_param_ex para;
         para.sched_priority = taskInfo.priority;
         /* sched_param
         para.sched_flags= 0;
         para.sched_runtime = taskInfo.periodicity;;
         para.sched_deadline = taskInfo.periodicity;
         para.sched_period = taskInfo->periodicity;
         para.size=sizeof(sched_attr);
         old... */
         if( (ret = sched_setscheduler_ex(curtaskinfo.pid, schedPolicy, &para)) != 0)
         {
            cerr << "[" << taskInfo.name << "] " << "error setting scheduling policy " << schedPolicy << ", Error #" << ret;
            exit(ret);
         }
         //Periodicity
         if ((ret = rt_task_set_periodic(taskInfo.task, TM_NOW, taskInfo.periodicity)))
         { cerr << "[" << taskInfo.name << "] " << "Set_Period Error : " << ret << " ." << endl; exit(-2); }
         rt_task_affinity(taskInfo.task, taskInfo.affinity, 0);
         if (schedPolicy == SCHED_RR)
         { //#if defined SCHED_POLICY  &&  SCHED_POLICY == SCHED_RR
            if ((ret = rt_task_slice(taskInfo.task, RR_SLICE_TIME)))
            { cerr << "[" << taskInfo.name << "] " << "Slice Error : " << ret << " ." << endl; exit(-4); }
         } //#endif
         if ((ret = rt_task_set_priority(taskInfo.task, taskInfo.priority)))
            { cerr << "[" << taskInfo.name << "] " << "Set_Priority Error : " << ret << " ." << endl; exit(-5); }
         /* Gestion EDF Scheduling
         RT_TASK_INFO curtaskinfo;
         rt_task_inquire(taskInfo->task, &curtaskinfo);

         struct sched_attr para;
         para.sched_policy = SCHED_POLICY;
         para.sched_flags= 0;
         //para.sched_runtime = taskInfo.periodicity;;
         //para.sched_deadline = taskInfo.periodicity;
         para.sched_period = taskInfo->periodicity;
         para.sched_priority = taskInfo->priority;
         para.size=sizeof(sched_attr);
         rt_task_inquire(taskInfo->task, &curtaskinfo);
         if( sched_setattr(curtaskinfo.pid, &para, 0) != 0)
         {
            fprintf(stderr,"error setting scheduler ... are you root? : %d \n", errno);
            exit(errno);
         }
*/
      }

   }
   return 0;
}

int TaskLauncher::runTasks()
{
   #if VERBOSE_INFO
   cout << endl << "STARTING TASKS : " << endl;
   #endif

   for (auto& taskInfo : taskSetInfos.rtTIs)
   {
      TaskDataLogger* dlog = new TaskDataLogger(&taskInfo);
      taskRTInfo* _taskRTInfo = new taskRTInfo;
      _taskRTInfo->taskLog = dlog;
      _taskRTInfo->rtTI = &taskInfo;
      //printInquireInfo(taskInfo.task);
      if( rt_task_start(taskInfo.task, TaskMain, _taskRTInfo))
      {
         cerr << "[" << taskInfo.name << "] :" << "Failed to start task." << endl;
         return -1;
      }
      else
      {
         #if VERBOSE_INFO
         cout << "Task " << taskInfo.name << " running." << endl;
         #endif
         tasksLogsList.push_back(dlog);
      }
   }
   return 0;
}

int TaskLauncher::runAgent()
{
  int ret = 0;
   #if VERBOSE_INFO
   cout << endl << "LAUNCHING MoCoAgent." << endl;
   #endif
   if( (ret = rt_task_create(&mcAgent, "MoCoAgent", 0, 99, 0)) )
   {
      cerr << "Error creating Monitoring and Control Agent (" << ret << ")." << endl;
      return -1;
   }
   else
   {
        enableAgent = 1;
        rt_task_affinity(&mcAgent, 0, 0);
        //if ( (ret = rt_task_set_periodic(&mcAgent, TM_NOW, 5*1e6)) )
      //      { cerr << "[MoCoAgent] " << "Set_Period Error : " << ret << " ." << endl; exit(-3); }
        //  systemRTInfo ch_taks ;
        if ( (ret = rt_task_start(&mcAgent, RunmcAgentMain, &taskSetInfos)) )
        {
           cerr << "[ Task Launcher ] :" << "Failed to start Mo Co Agent. ("<< ret << ")."  << endl;
           return -1;
        }
   }
   return 0;
}

void TaskLauncher::stopTasks(bool val)
{
   if (val)
   {
      if (enableAgent) rt_task_suspend(&mcAgent);
      for (auto& task : taskSetInfos.rtTIs)
      {
         rt_task_suspend(task.task);
      }
   }
   else
   {
      if (enableAgent) rt_task_resume(&mcAgent);
      for (auto& task : taskSetInfos.rtTIs)
      {
         rt_task_resume(task.task);
      }
   }
}


void TaskLauncher::saveData(string file)
{
   cout << "Stopping Tasks." << endl;
   stopTasks(1);
   sleep (1);
   cout << "Saving tasks data..." << endl;
   //cout << "Checking tasks names :" << endl;
   int nameMaxSize = 0;
   for (auto& taskLog : tasksLogsList)
   {
      int sizeName = strlen(taskLog->getName());
      //cout << taskLog->getName() << " - size is : " << sizeName << endl;
      if (sizeName > nameMaxSize) nameMaxSize = sizeName;
   }
   //cout << "Max name size is : " << nameMaxSize << endl;

   std::ofstream myFile;
   string dataFileName = file + "_Expe.csv";
   myFile.open (dataFileName);    // TO APPEND :  //,ios_base::app);
   myFile << std::setw(15) << "timestamp" << " ; "
          << std::setw(nameMaxSize) << "name"     << " ; "
          << std::setw(2)  << "ID"       << " ; "
          << std::setw(3)  << "HRT"      << " ; "
          << std::setw(4) << "Prio"      << " ; "
          << std::setw(10) << "deadline" << " ; "
          << std::setw(4)  << "aff."     << " ; "
          << std::setw(10) << "duration" << "\n";
   myFile.close();
   for (auto& taskLog : tasksLogsList)
   {
      taskLog->saveData(file, nameMaxSize);
   }

   if (enableAgent)
   {
     triggerSave = 1;
     rt_task_resume(&mcAgent);
     sleep (1);
   }

}

void TaskLauncher::rt_task_affinity (RT_TASK* task, int _aff, int mode)
{ // mode 0 : replace | mode 1 : add | mode -1 : remove
   cpu_set_t mask;
   if (mode == 0) { CPU_ZERO(&mask); CPU_SET(_aff, &mask); }
   else if (mode > 1) CPU_SET(_aff, &mask);
   else if (mode < -1) CPU_CLR(_aff, &mask);

   RT_TASK_INFO curtaskinfo;
   rt_task_inquire(task, &curtaskinfo);

   if (int ret = rt_task_set_affinity(task, &mask))
   {
      cerr << "Error while setting (" << mode << ") affinity for task "
      << curtaskinfo.name << " to CPU " << _aff << ": " <<  ret << "."<< endl;
   }
   #if VERBOSE_ASK
   if (mode == 0) cout << "Switched affinity for task " << curtaskinfo.name << " = CPU " << _aff << endl;
   else if (mode == 1) cout << "Added affinity for task " << curtaskinfo.name << " +CPU " << _aff << endl;
   else if (mode == -1) cout << "Removed affinity for task " << curtaskinfo.name << " -CPU " << _aff << endl;
   #endif
}

void TaskLauncher::printTasksInfos ( ) // std::vector<rtTaskInfosStruct> _myTasksInfos)
{
   #if VERBOSE_INFO
   cout << "Resume of tasks set information : " << endl;
   for (auto &taskInfo : taskSetInfos.rtTIs)
   {
      cout << "Name: " << taskInfo.name
      << "| path: " << taskInfo.path_task
      << "| is RT ? " << taskInfo.isHardRealTime
      << "| Period: " << taskInfo.periodicity/1.0e6
      << "| Deadline: " << taskInfo.wcet/1.0e6
      << "| affinity: " << taskInfo.affinity
      << "| priority: " << taskInfo.priority
      << "| ID :"<< taskInfo.id << endl;

   }
   #endif
}

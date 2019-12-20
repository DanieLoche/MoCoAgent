#include "tools.h"
#include "taskLauncher.h"
#include "macroTask.h"
#include "sched.h"
#include "edf.h"

#include <iomanip>


TaskLauncher::TaskLauncher(string _outputFileName, int _schedMode)
{
   enableAgent = 0;
   schedPolicy = _schedMode;

   bool trigA = new bool();
   trigA = 0;
   triggerSave = trigA;
   triggerSave = &triggerSave;
   outputFileName = _outputFileName;
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
         e2eDD.push_back(chaineInfo);
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
   for(int i=0; i < (int)e2eDD.size(); ++i )
   {
      std::ifstream myFile(e2eDD[i].Path);
      if (!myFile.is_open())
      {
         cerr << "Failed to open file : " << e2eDD[i].Path << endl;
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
                      >> taskInfo->function ) )
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

            rtTIs.push_back(*taskInfo);
         }
         #if VERBOSE_ASK
         else cout << "line ignored." << endl;
         #endif
      }

      if (schedPolicy == SCHED_RM)
      { // Changer les niveaux de priorité si on schedule en RM.
         cout << "Updating task informations to use Rate-Monotinic Scheduling." << endl;
         std::sort(rtTIs.begin(), rtTIs.end(), sortAscendingPeriod());
         int prio = 10;
         RTIME lastPeriod = rtTIs[0].periodicity;
         for (auto taskInfo = rtTIs.begin(); taskInfo != rtTIs.end(); ++taskInfo)
         {
            if (taskInfo->periodicity != lastPeriod)
            {
               prio += 2;
               lastPeriod = taskInfo->periodicity;
            }
            taskInfo->priority = prio;
         }
      }
   }
   return 0;
}

/* createMutexes
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

   #if VERBOSE_INFO
   cout << endl << "CREATING TASKS : " << endl;
   #endif
   //for (auto taskInfo = taskSetInfos.rtTIs.begin(); taskInfo != taskSetInfos.rtTIs.end(); ++taskInfo)
   for (auto& taskInfo : rtTIs)
   {
      #if VERBOSE_INFO
      cout << "Creating Task " << taskInfo.name << "." << endl;
      #endif

      pid_t pid = fork();
      if (pid == 0) // proc fils
      {
         currentTaskDescriptor = taskInfo;
         //TaskDataLogger* dlog = new TaskDataLogger(&currentTaskDescriptor);
         //taskRTInfo* _taskRTInfo = new taskRTInfo;
         //_taskRTInfo->taskLog = dlog;
         //_taskRTInfo->rtTI = &currentTaskDescriptor;

         MacroTask macroRT(taskInfo, enableAgent);
         rt_sem_p(&mysync,TM_INFINITE);
         sleep(1);
         if (currentTaskDescriptor.isHardRealTime == 0) {
           macroRT.executeRun_besteffort();
         }
         else {
           macroRT.executeRun();
         }

      }
      else // pid = forked task pid_t
      {
         tasks.push_back(new RT_TASK);
         rt_task_bind(tasks.back(), taskInfo.name, TM_INFINITE); // A faire dans MoCoAgent ?
      }

   }
   return 0;
}

int TaskLauncher::runTasks()
{
   #if VERBOSE_INFO
   cout << endl << "STARTING TASKS : " << endl;
   #endif

   // for (auto& taskInfo : taskSetInfos.rtTIs)
   {
      /* Gestion DataLogger à Changer
      TaskDataLogger* dlog = new TaskDataLogger(&currentTaskDescriptor);
      taskRTInfo* _taskRTInfo = new taskRTInfo;
      _taskRTInfo->taskLog = dlog;
      _taskRTInfo->rtTI = &currentTaskDescriptor;

      //printInquireInfo(currentTaskDescriptor.task);
      if( rt_task_start(currentTaskDescriptor.task, TaskMain, _taskRTInfo))
      {
         cerr << "[" << currentTaskDescriptor.name << "] :" << "Failed to start task." << endl;
         return -1;
      }
      else
      {
         #if VERBOSE_INFO
         cout << "Task " << currentTaskDescriptor.name << " running." << endl;
         #endif
         tasksLogsList.push_back(dlog);
      }
     */

      rt_task_resume(currentTaskDescriptor._t);
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
      for (auto& task : rtTIs)
      {
         rt_task_suspend(task._t);
      }
   }
   else
   {
      if (enableAgent) rt_task_resume(&mcAgent);
      for (auto& task : rtTIs)
      {
         rt_task_resume(task._t);
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
     sleep (2);
   }

   for (auto& task : rtTIs)
   {
      rt_task_delete(task._t);
   }

}

void TaskLauncher::printTasksInfos ( ) // std::vector<rtTaskInfosStruct> _myTasksInfos)
{
   #if VERBOSE_INFO
   cout << "Resume of tasks set information : " << endl;
   for (auto &taskInfo : rtTIs)
   {
      cout << "Name: " << taskInfo.name
      << "| path: " << taskInfo.function
      << "| is RT ? " << taskInfo.isHardRealTime
      << "| Period: " << taskInfo.periodicity/1.0e6
      << "| Deadline: " << taskInfo.wcet/1.0e6
      << "| affinity: " << taskInfo.affinity
      << "| priority: " << taskInfo.priority
      << "| ID :"<< taskInfo.id << endl;

   }
   #endif
}

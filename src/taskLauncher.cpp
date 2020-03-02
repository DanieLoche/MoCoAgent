#include "taskLauncher.h"


string TaskLauncher::outputFileName = "";
int TaskLauncher::nameMaxSize = 32;
std::vector<rtTaskInfosStruct> TaskLauncher::tasksSet = std::vector<rtTaskInfosStruct>();
std::vector<RT_TASK*> TaskLauncher::tasks = std::vector<RT_TASK*>();
RT_SEM TaskLauncher::_syncSem = {0};

TaskLauncher::TaskLauncher(int _agentMode, string _outputFileName, int _schedMode)
{
   enableAgent = _agentMode;
   schedPolicy = _schedMode;

   //triggerSave = FALSE;
   //triggerSave = &triggerSave;
   outputFileName = _outputFileName;
   nameMaxSize = 0;
}


int TaskLauncher::readChainsList(string input_file)
{
   cout << "====== READING CHAINS FILE ======"<< endl;

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
      cout << "Managing line : " << str;
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
      else cout << " ==> line ignored.";
      #endif
       cout << endl;
   }

   printChainSetInfos ( );
   return 0;
}

int TaskLauncher::readTasksList(int cpuPercent)
{
   float cpuFactor = cpuPercent/100.0;
   #if VERBOSE_ASK
   cout << "====== READING TASKS FILE ======"<< endl;
   cout << "CPU use factor = " << cpuFactor <<  endl;
   #endif
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
         cout << "Managing line : " << str;
         #endif
         if (str.substr(0,2) != "//")
         {
            float tmp_wcet = 0;
            int tmp_period = 0;
            char name[28];
            if (!(iss >> taskInfo->fP.id >> name
                      >> tmp_wcet     // WCET -> for task chain // placeholder.
                      >> tmp_period   // meanET -> period
                      >> taskInfo->fP.isHRT
                      >> taskInfo->rtP.affinity
                      >> taskInfo->fP.prec
                      >> taskInfo->fP.func ) )
            { cerr << "Failed to read line : " << str << endl; return -1; } // error
            //taskInfo->isHardRealTime = taskSetInfos.e2eDD[i].taskChainID;
            // Traitement du nom de la tâche
            strncat(ext, name, 32);
            strcpy(taskInfo->fP.name,ext);

            getline(iss , taskInfo->fP.args);
            taskInfo->fP.args = reduce(taskInfo->fP.args);

            taskInfo->fP.wcet = tmp_wcet * 1.0e6;              // conversion ms to RTIME (ns)
            taskInfo->rtP.priority = 50;
            // Traitement de la périodicité de la tâche
            taskInfo->rtP.periodicity = cpuFactor * _mSEC(tmp_period); //taskInfo->periodicity = taskInfo->periodicity * 1.0e6 * cpuFactor;
            //printTaskInfo(&taskInfo); // Résumé
            taskInfo->rtP.schedPolicy = schedPolicy;

            tasksSet.push_back(*taskInfo);
         }
         #if VERBOSE_ASK
         else cout << " ==> line ignored.";
         #endif
         cout << endl;
      }

      if (schedPolicy == SCHED_RM)
      { // Changer les niveaux de priorité si on schedule en RM.
         #if VERBOSE_INFO
         cout << "Updating task informations to use Rate-Monotinic Scheduling." << endl;
         #endif
         std::sort(tasksSet.begin(), tasksSet.end(), sortAscendingPeriod());
         int prio = 10;
         RTIME lastPeriod = tasksSet[0].rtP.periodicity;
         for (auto taskInfo = tasksSet.begin(); taskInfo != tasksSet.end(); ++taskInfo)
         {
            if (taskInfo->rtP.periodicity != lastPeriod)
            {
               prio += 2;
               lastPeriod = taskInfo->rtP.periodicity;
            }
            taskInfo->rtP.priority = prio;
         }
      }

   }

   printTaskSetInfos();

   return 0;
}

int TaskLauncher::runTasks(long expeDuration)
{
   #if VERBOSE_INFO
   cout << endl << "====== LAUNCHING TASKS ======" << endl;
   #endif
   //for (auto taskInfo = taskSetInfos.rtTIs.begin(); taskInfo != taskSetInfos.rtTIs.end(); ++taskInfo)

   for (auto& taskInfo : tasksSet)
   {
      #if VERBOSE_INFO
      cout << "Creating Task " << taskInfo.fP.name << "." << endl;
      #endif
      pid_t pid = fork();
      if (pid == 0) // proc fils
      {
         setvbuf(stdout, NULL, _IOLBF, 4096) ;
         currentTaskDescriptor = taskInfo;
         //RT_TASK _t;
         //ERROR_MNG(rt_task_shadow(&_t, "TOTO", 1, 0));
         //sleep(50);
         currentProcess = new MacroTask(currentTaskDescriptor, enableAgent, outputFileName);

         #if VERBOSE_DEBUG
         //rt_printf("[ %s ] - Process created (pid = %d).\n", currentTaskDescriptor.fP.name, getpid());
         #endif

         ERROR_MNG(rt_alarm_create(&_endAlarm, ALARM_NAME, TaskLauncher::finishTask, (void*) currentProcess)); //ms to ns
         #if VERBOSE_DEBUG
         //rt_printf("[ %s ] - Alarm %s created.\n", currentTaskDescriptor.fP.name, ALARM_NAME);
         #endif

         ERROR_MNG(rt_sem_bind(&_syncSem, SEM_NAME, TM_INFINITE)); // Wait Semaphor created.
         #if VERBOSE_DEBUG
         //rt_printf("[ %s ] - Semaphor %s joined.\n", currentTaskDescriptor.fP.name, SEM_NAME);
         #endif

         rt_alarm_start(&_endAlarm, _SEC(expeDuration), TM_INFINITE);
         #if VERBOSE_DEBUG
         //rt_printf("[ %s ] - Alarm %s set.\n", currentTaskDescriptor.fP.name, ALARM_NAME);
         #endif

         ERROR_MNG(rt_sem_v(&_syncSem)); // Signal that this task is ready.
         #if VERBOSE_DEBUG
         //rt_printf("[ %s ] - Semaphor %s released !\n", currentTaskDescriptor.fP.name, SEM_NAME);
         #endif

         rt_task_sleep(_mSEC(10));
         //rt_task_wait_period(0);
         ERROR_MNG(rt_sem_p(&_syncSem, TM_INFINITE)); // Wait broadcast to run.
         #if VERBOSE_DEBUG
         rt_fprintf(stderr, "[ %s ] - Semaphor %s signal received, go !\n", currentTaskDescriptor.fP.name, SEM_NAME);
         #endif
         //rt_task_sleep(_mSEC(10));
         if (currentTaskDescriptor.fP.isHRT == 0) {
            //rt_fprintf(stderr, "[ %s ] - Execution in progress - BE.\n", currentTaskDescriptor.fP.name);
            rt_print_flush_buffers();

            currentProcess->executeRun_besteffort();
         }
         else {
            //rt_fprintf(stderr, "[ %s ] - Execution in progress - CT.\n", currentTaskDescriptor.fP.name);
            rt_print_flush_buffers();

            currentProcess->executeRun();
         }
         exit(EXIT_FAILURE); // should never occur.
      }
      else // pid = forked task pid_t
      {
         //RT_TASK _t;
         //int ret = rt_task_bind(&_t, taskInfo.fP.name, _mSEC(500));
         //cout << "Waiting to bind to " << taskInfo.fP.name << "..." << getErrorName(ret) << " (" << ret << ")" << endl;
         sleep(0.5);
      }
   }
   return 0;
}

int TaskLauncher::runAgent(long expeDuration)
{
   int ret = 0;
   #if VERBOSE_INFO
   cout << "====== LAUNCHING MoCoAgent ======" << endl;
   #endif

   rtTaskInfosStruct MoCoAgentParams = {
      0,          // Affinity
      _mSEC(MCA_PERIOD), // periodicity
      98,         // Priority
      SCHED_FIFO, // Scheduling POLICY
      99,         // id
      "MoCoAgent", // char[32] name
      "",         // function
      " > "+outputFileName,    // stdOut will be used for the log file name here ..!
      99,0,0,     // isHRT/task chain ID,precedency & WCET.
   };
   currentTaskDescriptor = MoCoAgentParams;
   printTaskInfo(&currentTaskDescriptor);

   //printf("dumping the registry\n") ;
   //sleep(2);
   //system("find /run/xenomai") ; // see what the registry is looking like
   cout << std::flush;
   currentProcess = new MCAgent(MoCoAgentParams, e2eDD, tasksSet);
   rt_printf("[ %s ] - Process created (pid = %d).\n", currentTaskDescriptor.fP.name, getpid()); //cout << "["<< currentTaskDescriptor.fP.name << "]"<< "Macro task created." << endl;
   for (auto& taskInfo : tasksSet)
   {
      RT_TASK* _t = new RT_TASK;
      ERROR_MNG(rt_task_bind(_t, taskInfo.fP.name, _mSEC(500)));
      tasks.push_back(_t);
   }
   //sleep(50);

   ERROR_MNG(rt_alarm_create(&_endAlarm, ALARM_NAME, TaskLauncher::finishMoCoAgent, (void*)currentProcess)); //ms to ns
   #if VERBOSE_DEBUG
   rt_printf("[ %s ] - Alarm %s created.\n", currentTaskDescriptor.fP.name, ALARM_NAME); //cout << "["<< currentTaskDescriptor.fP.name << "]"<< "Alarm created." << endl;
   #endif

   ERROR_MNG(rt_sem_create(&_syncSem, SEM_NAME, 0, S_PRIO)); // sync. #1: ready for alarms.
   #if VERBOSE_DEBUG
   rt_printf("[ %s ] - Semaphor %s created.\n", currentTaskDescriptor.fP.name, SEM_NAME); //cout << "["<< currentTaskDescriptor.fP.name << "]"<< "Semaphor Created." << endl;
   #endif

   //rt_task_wait_period(0);
   rt_alarm_start(&_endAlarm, _SEC(expeDuration), TM_INFINITE);
   #if VERBOSE_DEBUG
   rt_printf("[ %s ] - Alarm %s set.\n", currentTaskDescriptor.fP.name, ALARM_NAME); //cout << "["<< currentTaskDescriptor.fP.name << "]"<< "Alarm set." << endl;
   #endif

   for (auto taskInfo = tasksSet.begin(); taskInfo != tasksSet.end(); ++taskInfo)
   { // waiting every alarm is set.
      rt_sem_p(&_syncSem, TM_INFINITE); // => wait for a semaphor release from every task.
      #if VERBOSE_DEBUG
      rt_printf("[ %s ] - Semaphor %s catched !\n", currentTaskDescriptor.fP.name, SEM_NAME); //cout << "["<< currentTaskDescriptor.fP.name << "]"<< "Semaphor catched !" << endl;
      #endif

   }
   rt_task_sleep(_mSEC(20));
   #if VERBOSE_INFO
   rt_printf("[ MoCoAgent ] - GO !\n");
   #endif

   rt_sem_broadcast(&_syncSem); // Alarms OK. Start Run !
   //rt_printf("[%llu][ %s ] - semaphor %s Broadcasted.\n", rt_timer_read(), currentTaskDescriptor.fP.name, SEM_NAME); //cout << "["<< currentTaskDescriptor.fP.name << "]"<< "Semaphor Released (broadcast !)." << endl;
   rt_task_wait_period(0);
   if (enableAgent == 2) currentProcess->executeRun_besteffort();
   else currentProcess->executeRun();

   ret = 1;
   for (auto& _task : tasks)
   {
      while (ret)
      {
         ret = rt_task_inquire(_task, NULL);
         //rt_task_sleep(_mSEC(2));
      }
   }
   rt_sem_p(&_syncSem, TM_INFINITE);

   cout << " ======= END OF EXPERIMENTATION ======" << endl;
   exit(EXIT_SUCCESS);
   return ret;
}

void TaskLauncher::finishMoCoAgent(void* _arg)
{
   MCAgent* MoCoAgent_task = (MCAgent*) _arg;

   cout << "Saving Data..." << endl;
   //cout << "Checking tasks names :" << endl;
   if (!tasksSet.empty())
      for (auto taskInfo = tasksSet.begin(); taskInfo != tasksSet.end(); ++taskInfo)
      {
         int sizeName = strlen(taskInfo->fP.name);
         if (sizeName > nameMaxSize) nameMaxSize = sizeName;
      }
      else cerr << "WOAW !! Task set is empty !!" << endl;
   std::ofstream myFile;
   myFile.open (outputFileName + TASKS_FILE);    // TO APPEND :  //,ios_base::app);
   myFile << std::setw(15) << "timestamp" << " ; "
         << std::setw(nameMaxSize) << "name"     << " ; "
         << std::setw(2)  << "ID"       << " ; "
         << std::setw(3)  << "HRT"      << " ; "
         << std::setw(4) << "Prio"      << " ; "
         << std::setw(10) << "deadline" << " ; "
         << std::setw(4)  << "aff."     << " ; "
         << std::setw(10) << "duration" << "\n";
   myFile.close();

   MoCoAgent_task->saveData();

   RTIME time = rt_timer_read();
   rt_sem_v(&_syncSem);
   rt_fprintf(stderr, "[ %llu ] [ MoCoAgent ]- Giving Semaphor...\n", time);
   rt_print_flush_buffers();

   MoCoAgent_task->MoCoIsAlive = 0;
   //ERROR_MNG(rt_task_sleep(_mSEC(500)));
}

void TaskLauncher::finishTask(void* _MacroTask)
{
   MacroTask* task = (MacroTask*) _MacroTask;

   rt_fprintf(stderr, "[ %llu ][ %s ] - Waiting Semaphor...\n", rt_timer_read(), task->prop.fP.name);

   rt_sem_p(&_syncSem, TM_INFINITE);
   rt_fprintf(stderr, "[ %llu ][ %s ] - Got a Semaphor...\n", rt_timer_read(), task->prop.fP.name);

      task->saveData(nameMaxSize);

   RTIME time = rt_timer_read();
   rt_sem_v(&_syncSem);
   rt_fprintf(stderr, "[ %llu ][ %s ] - Semaphor released.\n", time, task->prop.fP.name);

   rt_fprintf(stderr, "[ %llu ][ %s ] - Finished.\n", time, task->prop.fP.name);
   rt_print_flush_buffers();

   cerr << "Task " << task->prop.fP.name << " Finished." << endl;

   //ERROR_MNG(rt_task_sleep(_SEC(1)));
   exit(EXIT_SUCCESS);
}


void TaskLauncher::printTaskSetInfos ( ) // std::vector<rtTaskInfosStruct> _myTasksInfos)
{
   #if VERBOSE_INFO
   cout << "Resume of tasks set information : " << endl;
   if (!tasksSet.empty())
      for (auto &taskInfo : tasksSet)
      {
         cout << "Name: " << taskInfo.fP.name
         << "  | path: " << taskInfo.fP.func
         << "  | is RT ? " << taskInfo.fP.isHRT
         << "  | Period: " << taskInfo.rtP.periodicity/1.0e6
         << "  | Deadline: " << taskInfo.fP.wcet/1.0e6
         << "  | affinity: " << taskInfo.rtP.affinity
         << "  | priority: " << taskInfo.rtP.priority
         << "  | ID: "<< taskInfo.fP.id << endl;

      }
   else cerr << "[ERROR] - No Tasks to display !" << endl;
   #endif
}

void TaskLauncher::printChainSetInfos ( ) // std::vector<rtTaskInfosStruct> _myTasksInfos)
{
   #if VERBOSE_INFO
   cout << "Resume of Chain set information : " << e2eDD.size() << " elements." << endl;
   if (!e2eDD.empty())
      for (auto &chainInfo : e2eDD)
      {
         cout << "Name: " << chainInfo.name
         << "  | ID: "<< chainInfo.taskChainID
         << "  | path: " << chainInfo.Path
         << "  | Deadline: " << chainInfo.deadline << endl;
      }
   else cerr << "[ERROR] - No Task Chain to display !" << endl;
   #endif
}



/*
void TaskLauncher::stopTasks(bool val)
{
   if (val)
   {
      if (enableAgent) rt_task_suspend(&(currentProcess->_task));
      for (auto& task : tasks)
      {
         rt_task_suspend(task);
      }
   }
   else
   {
      if (enableAgent) rt_task_resume(&(currentProcess->_task));
      for (auto& task : tasks)
      {
         rt_task_resume(task);
      }
   }
}
*/

/*
void TaskLauncher::saveData(string file)
{
   cout << "Stopping Tasks." << endl;
   stopTasks(1);
   sleep (1);
   cout << "Saving tasks data..." << endl;
   //cout << "Checking tasks names :" << endl;
   for (auto taskInfo = tasksSet.begin(); taskInfo != tasksSet.end(); ++taskInfo)
   {
      int sizeName = strlen(taskInfo->fP.name);
      if (sizeName > nameMaxSize) nameMaxSize = sizeName;
   }
   //cout << "Max name size is : " << nameMaxSize << endl;

   std::ofstream myFile;
   myFile.open (file + "_Expe.csv");    // TO APPEND :  //,ios_base::app);
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
      rt_task_resume(currentProcess->_task);
      sleep (2);
   }

   for (auto& task : tasks)
   {
      rt_task_delete(task);
   }

}
*/

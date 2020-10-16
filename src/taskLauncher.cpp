#include "taskLauncher.h"


string TaskLauncher::outputFileName = "";
int TaskLauncher::nameMaxSize = 0;
std::vector<rtTaskInfosStruct> TaskLauncher::tasksSet = std::vector<rtTaskInfosStruct>();
std::vector<end2endDeadlineStruct> TaskLauncher::chainSet = std::vector<end2endDeadlineStruct>();


TaskLauncher::TaskLauncher(int _agentMode, string _outputFileName, int _schedMode)
{
   enableAgent = _agentMode;
   schedPolicy = _schedMode;
   outputFileName = _outputFileName;

   //triggerSave = FALSE;
   //triggerSave = &triggerSave;
   //nameMaxSize = 32;
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
         chainSet.push_back(chaineInfo);
      }
      #if VERBOSE_ASK
      else cout << " ==> line ignored.";
       cout << endl;
       #endif
   }

   chainSet.shrink_to_fit();
   if (!chainSet.empty())
   {
      printChainSetInfos ( );
      return 0;
   } else return -1;
}

int TaskLauncher::readTasksList(int cpuPercent)
{
   float cpuFactor = cpuPercent/100.0;
   #if VERBOSE_ASK
   cout << "====== READING TASKS FILE ======"<< endl;
   cout << "CPU use factor = " << cpuFactor <<  endl;
   #endif
   for(int i=0; i < (int) chainSet.size(); ++i )
   {
      std::ifstream myFile(chainSet[i].Path);
      if (!myFile.is_open())
      {
         cerr << "Failed to open file : " << chainSet[i].Path << endl;
         exit(EXIT_FAILURE);
      }
      string str;
      std::getline(myFile, str); // skip the first line
      while (std::getline(myFile, str))
      {
         rtTaskInfosStruct* taskInfo = new rtTaskInfosStruct;
         std::istringstream iss(str);
         string token;
         char ext[32] = "";
         #if VERBOSE_ASK
         cout << "Managing line : " << str;
         #endif
         if (str.substr(0,2) != "//")
         {
            float tmp_wcet = 0;
            int tmp_period = 0, tmp_HRT = 0;
            char name[28];
            if (!(iss >> taskInfo->fP.id >> name
                      >> tmp_wcet     // WCET -> for task chain // placeholder.
                      >> tmp_period   // meanET -> period
                      >> tmp_HRT
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

            taskInfo->fP.wcet = _uSEC(tmp_wcet);              // conversion us to RTIME (ns)
            taskInfo->rtP.priority = abs(tmp_HRT);
            taskInfo->fP.isHRT = std::max(0,sign(tmp_HRT)); //0 for BE, 1 for HRT
            // Traitement de la périodicité de la tâche
            taskInfo->rtP.periodicity = cpuFactor * _mSEC(tmp_period); //taskInfo->periodicity = taskInfo->periodicity * 1.0e6 * cpuFactor;
            //printTaskInfo(&taskInfo); // Résumé
            taskInfo->rtP.schedPolicy = schedPolicy;

            tasksSet.push_back(*taskInfo);
         }
         #if VERBOSE_ASK
         else cout << " ==> line ignored.";
         cout << endl;
         #endif
      }

      if (tasksSet.empty())
      {
         return -1;
      }

      if (schedPolicy == SCHED_RM)
      { // Changer les niveaux de priorité si on schedule en RM.
         #if VERBOSE_INFO
         cout << "Updating task informations to use Rate-Monotinic Scheduling." << endl;
         #endif
         std::sort(tasksSet.begin(), tasksSet.end(), sortAscendingPeriod());
         int prio = 2;
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

   for (auto taskInfo = tasksSet.begin(); taskInfo != tasksSet.end(); ++taskInfo)
   {
      int sizeName = strlen(taskInfo->fP.name);
      if (sizeName > nameMaxSize) nameMaxSize = sizeName;
   }

   tasksSet.shrink_to_fit();
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
         //setvbuf(stdout, NULL, _IOLBF, 4096) ; // _IONBF
         //setvbuf(stderr, NULL, _IOLBF, 4096) ; // _IOLBF

         currentTaskDescriptor = taskInfo;
         MacroTask* currentProcess;

         //RT_TASK _t;
         //ERROR_MNG(rt_task_shadow(&_t, "TOTO", 1, 0));
         //sleep(50);
         if (currentTaskDescriptor.fP.isHRT == 0)
            currentProcess = new BEMacroTask(currentTaskDescriptor, enableAgent, outputFileName);
         else
            currentProcess = new RTMacroTask(currentTaskDescriptor, enableAgent, outputFileName);

         #if VERBOSE_DEBUG
         rt_fprintf(stderr, "[ %llu ][ %s ] - Process created (pid = %d).\n", rt_timer_read(), currentTaskDescriptor.fP.name, getpid());
         #endif

         ERROR_MNG(rt_alarm_create(&_endAlarm, ALARM_NAME, TaskLauncher::finishTask, (void*) currentProcess)); //ms to ns
         #if VERBOSE_DEBUG
         rt_fprintf(stderr, "[ %llu ][ %s ] - Alarm %s created.\n", rt_timer_read(), currentTaskDescriptor.fP.name, ALARM_NAME);
         #endif

         currentProcess->setCommunications();
         ERROR_MNG(rt_sem_bind(&_sync_MC_Sem, SEM_MC_NAME, TM_INFINITE)); // Wait Semaphor created.
         ERROR_MNG(rt_sem_bind(&_sync_Task_Sem, SEM_TASK_NAME, TM_INFINITE)); // Wait Semaphor created.
         #if VERBOSE_DEBUG
         rt_fprintf(stderr, "[ %llu ][ %s ] - Semaphors %s & %s joined.\n", rt_timer_read(), currentTaskDescriptor.fP.name, SEM_MC_NAME, SEM_TASK_NAME);
         #endif

         rt_alarm_start(&_endAlarm, _SEC(expeDuration), TM_INFINITE);
         #if VERBOSE_DEBUG
         rt_fprintf(stderr, "[ %llu ][ %s ] - Alarm %s set.\n", rt_timer_read(), currentTaskDescriptor.fP.name, ALARM_NAME);
         #endif

         ERROR_MNG(rt_sem_v(&_sync_Task_Sem)); // Signal that this task is ready.
         #if VERBOSE_DEBUG
         rt_fprintf(stderr, "[ %llu ][ %s ] - Semaphor %s released !\n", rt_timer_read(), currentTaskDescriptor.fP.name, SEM_TASK_NAME);
         #endif

         ERROR_MNG(rt_sem_p(&_sync_MC_Sem, TM_INFINITE)); // Wait broadcast to run.
         #if VERBOSE_DEBUG
         rt_fprintf(stderr, "[ %llu ][ %s ] - Semaphor %s signal received, go !\n", rt_timer_read(), currentTaskDescriptor.fP.name, SEM_MC_NAME);
         #endif
         //rt_task_sleep(_mSEC(10));

            //rt_fprintf(stderr, "[ %s ] - Execution in progress - BE.\n", currentTaskDescriptor.fP.name);
            rt_print_flush_buffers();
            //rt_task_sleep(_mSEC(100)); // on attend que le système se stabilise avant de lancer le Control.
            currentProcess->executeRun();

            rt_task_sleep(_mSEC(1));

///////////////////////////////////////////////////////////
/////////////// END OF EXPERIMENT /////////////////////////
         rt_fprintf(stderr, "[ %llu ][ %s ] - Waiting Semaphor...\n", rt_timer_read(), currentProcess->prop.fP.name);
         rt_print_flush_buffers();

         rt_sem_p(&_sync_Task_Sem, TM_INFINITE);
         rt_fprintf(stderr, "[ %llu ][ %s ] - Got MoCoAgent Semaphor...\n", rt_timer_read(), currentProcess->prop.fP.name);

         RT_TASK_INFO cti;
         rt_task_inquire(&currentProcess->_task, &cti);
         currentProcess->saveData(nameMaxSize, &cti);

         RTIME time = rt_timer_read();
         rt_sem_v(&_sync_Task_Sem);
         rt_fprintf(stderr, "[ %llu ][ %s ] - Semaphor released.\n", time, currentProcess->prop.fP.name);

         rt_fprintf(stderr, "[ %llu ][ %s ] - Finished.\n", time, currentProcess->prop.fP.name);
         rt_print_flush_buffers();
         exit(EXIT_SUCCESS);
      }
      else // pid = forked task pid_t
      {
         sleep(0.2);
      }
   }
   return 0;
}

int TaskLauncher::runAgent(long expeDuration)
{
   #if VERBOSE_INFO
   cout << "====== LAUNCHING MoCoAgent ======" << endl;
   #endif

   rtTaskInfosStruct MoCoAgentParams = {
      0,          // Affinity
      98,         // Priority
      SCHED_FIFO, // Scheduling POLICY
      MCA_PERIOD, // periodicity
      99,         // id
      99,0,0,     // isHRT/task chain ID,precedency & WCET.
      "MoCoAgent", // char[32] name
      "",         // function
      " > "+outputFileName,    // stdOut will be used for the log file name here ..!
   };
   currentTaskDescriptor = MoCoAgentParams;
   printTaskInfo(&currentTaskDescriptor);

   //printf("dumping the registry\n") ;
   //sleep(2);
   //system("find /run/xenomai") ; // see what the registry is looking like
   cout << std::flush;
   Agent* currentProcess;
   if (enableAgent == 2)
      currentProcess = new MonitoringControlAgent(MoCoAgentParams, chainSet, tasksSet);
   else if (enableAgent == 1)
      currentProcess = new MonitoringAgent(MoCoAgentParams, chainSet, tasksSet);
   else currentProcess = new Agent(MoCoAgentParams, chainSet, tasksSet);


   rt_fprintf(stderr, "[ %llu ][ %s ] - Process created (pid = %d).\n", rt_timer_read(), currentTaskDescriptor.fP.name, getpid()); //cout << "["<< currentTaskDescriptor.fP.name << "]"<< "Macro task created." << endl;
   //sleep(50);

   ERROR_MNG(rt_alarm_create(&_endAlarm, ALARM_NAME, TaskLauncher::finishMoCoAgent, (void*)currentProcess)); //ms to ns
   #if VERBOSE_DEBUG
   rt_fprintf(stderr, "[ %llu ][ %s ] - Alarm %s created.\n", rt_timer_read(), currentTaskDescriptor.fP.name, ALARM_NAME); //cout << "["<< currentTaskDescriptor.fP.name << "]"<< "Alarm created." << endl;
   #endif

   ERROR_MNG(rt_sem_create(&_sync_MC_Sem, SEM_MC_NAME, 0, S_FIFO)); // sync. #1: ready for alarms.
   ERROR_MNG(rt_sem_create(&_sync_Task_Sem, SEM_TASK_NAME, 0, S_FIFO)); // sync. #1: ready for alarms.
   #if VERBOSE_DEBUG
   rt_fprintf(stderr, "[ %llu ][ %s ] - Semaphors %s and %s created.\n", rt_timer_read(), currentTaskDescriptor.fP.name, SEM_MC_NAME, SEM_TASK_NAME); //cout << "["<< currentTaskDescriptor.fP.name << "]"<< "Semaphor Created." << endl;
   #endif

   //rt_task_wait_period(0);
   rt_alarm_start(&_endAlarm, _SEC(expeDuration), TM_INFINITE);
   #if VERBOSE_DEBUG
   rt_fprintf(stderr, "[ %llu ][ %s ] - Alarm %s set.\n", rt_timer_read(), currentTaskDescriptor.fP.name, ALARM_NAME); //cout << "["<< currentTaskDescriptor.fP.name << "]"<< "Alarm set." << endl;
   #endif

   for (auto taskInfo = tasksSet.begin(); taskInfo != tasksSet.end(); ++taskInfo)
   { // waiting every alarm is set.
      rt_sem_p(&_sync_Task_Sem, TM_INFINITE); // => wait for a semaphor release from every task.
      #if VERBOSE_DEBUG
      rt_fprintf(stderr, "[ %llu ][ %s ] - Semaphor %s catched !\n", rt_timer_read(), currentTaskDescriptor.fP.name, SEM_TASK_NAME); //cout << "["<< currentTaskDescriptor.fP.name << "]"<< "Semaphor catched !" << endl;
      #endif

   }
   //rt_task_sleep(_mSEC(20));
   #if VERBOSE_INFO
   rt_fprintf(stderr, "[ %llu ][ MoCoAgent ] - GO !\n", rt_timer_read());
   #endif
   rt_print_flush_buffers();

   rt_sem_broadcast(&_sync_MC_Sem); // Alarms OK. Start Run !
   rt_task_wait_period(0);

   currentProcess->executeRun();

///////////////////////////////////////////////////////////
/////////////// END OF EXPERIMENT /////////////////////////

   rt_printf("====== End of Experimentation. Saving Data. ======\n");
   rt_print_flush_buffers();

   RT_TASK_INFO cti;
   if (!rt_task_inquire(NULL, &cti))
   {
      std::ofstream outputFileResume;
      outputFileResume.open (outputFileName + RESUME_FILE, std::ios::app);    // TO APPEND :  //,ios_base::app);
      outputFileResume << "\n Monitoring and Control Agent Stats : \n"
                    << "Primary Mode execution time - " << cti.stat.xtime/1.0e6 << " ms."
                    << " Timeouts : " << cti.stat.timeout << "\n"
                    << "   Mode Switches - " << cti.stat.msw << "\n"
                    << "Context Switches - " << cti.stat.csw << "\n"
                    << "Cobalt Sys calls - " << cti.stat.xsc
                    << endl;
      outputFileResume.close();
   }

   rt_fprintf(stderr, "[ %llu ] - SAVING AGENT DATA.\n", rt_timer_read());
   rt_print_flush_buffers();

   /*std::vector<RT_TASK*> rtTasks;
   for (auto& taskInfo : tasksSet)
   {
      RT_TASK* _t;
      ERROR_MNG(rt_task_bind(_t, taskInfo.fP.name, _mSEC(500)));
      rtTasks.push_back(new RT_TASK(*_t));

      rtTasks.shrink_to_fit();
   }*/
   currentProcess->saveData();

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

   RTIME time = rt_timer_read();
   rt_sem_v(&_sync_Task_Sem);
   rt_fprintf(stderr, "[ %llu ] [ MoCoAgent ]- Giving Semaphor...\n", time);
   rt_print_flush_buffers();

   /*if (!rtTasks.empty())
   for (auto _task : rtTasks)
   {
      ERROR_MNG(rt_task_join(_task));
      //rt_task_sleep(_mSEC(5));
   } else rt_printf("ERROR ! No task set to cheeeck..!\n");
   rt_print_flush_buffers();*/

   rt_task_sleep(_SEC(1));
   rt_sem_p(&_sync_Task_Sem, TM_INFINITE);

   rt_printf(" ======= END OF EXPERIMENTATION ======\n");
   rt_print_flush_buffers();

   exit(EXIT_SUCCESS);
}

void TaskLauncher::finishMoCoAgent(void* _arg)
{
   Agent* MoCoAgent_task = (Agent*) _arg;
   MoCoAgent_task->EndOfExpe = TRUE;
   //ERROR_MNG(rt_task_sleep(_mSEC(500)));
}

void TaskLauncher::finishTask(void* _MacroTask)
{
   MacroTask* currentProcess = (MacroTask*) _MacroTask;
   currentProcess->EndOfExpe = TRUE;
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
   cout << "Resume of Chain set information : " << chainSet.size() << " elements." << endl;
   if (!chainSet.empty())
      for (auto &chainInfo : chainSet)
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

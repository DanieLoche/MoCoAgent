#include <iomanip>
#include "dataLogger.h"
//#include "NanoLog.h"

DataLogger::DataLogger(string expeName){

   outputFileName = expeName;
   cptOutOfDeadline = 0;
   cptExecutions = 0;
   overruns = 0;

}

ChainDataLogger::ChainDataLogger(end2endDeadlineStruct _chainInfos, string expeName) : DataLogger(expeName)
{
   chainInfos = _chainInfos;
   deadline = _chainInfos.deadline;
   cptAnticipatedMisses = 0;
   cptWCET = 0;

   //nanolog::initialize(nanolog::GuaranteedLogger(), "./", expeName + "_chains", 1);
}

TaskDataLogger::TaskDataLogger(rtTaskInfosStruct _taskInfos, string expeName) : DataLogger(expeName)
{
   taskInfos = _taskInfos;
   deadline = _taskInfos.rtP.periodicity;
   execLogs = {{0}};

   // nanolog::initialize(nanolog::GuaranteedLogger(), "./", expeName + "_expe", 1);
   // LOG_INFO << "This is a Information Log test !";
   // LOG_WARN << "This is a WARNING log.";
   // LOG_CRIT << "This is a CRITical log message.";
}

void TaskDataLogger::logStart(RTIME startTime)
{
   execLogs[cptExecutions & MASK_SIZE].timestamp = startTime;
}


RTIME TaskDataLogger::logStart()
{
   return (execLogs[cptExecutions & MASK_SIZE].timestamp = rt_timer_read());
}

void TaskDataLogger::logExec(RTIME endTime)
{
   if (execLogs[cptExecutions & MASK_SIZE].timestamp)
   {
      execLogs[cptExecutions & MASK_SIZE].duration = endTime - execLogs[cptExecutions & MASK_SIZE].timestamp;

      if(execLogs[cptExecutions & MASK_SIZE].duration > deadline )
      {
         cptOutOfDeadline++;
         #if VERBOSE_ASK
         rt_fprintf(stderr, "[ Warning ] [ %s ] - Executed in %.2f ms.\n",getName(),execLogs[cptExecutions & MASK_SIZE].duration/1e6);
         #endif
      }else{
         #if VERBOSE_ASK
         //fprintf(stderr, "[ \033[1;32mPERFECT\033[0m ] Task : \033[1;32m%s\033[0m - \033[1;36m%.2f ms\033[0m\n",getName(),execLogs[cptExecutions].duration/1e6);
         #endif
      }
      cptExecutions++;
      //return execLogs[cptExecutions - 1].duration;
      if (cptExecutions == BUFF_SIZE)
      {
         rt_fprintf(stderr, "[ WARNING ][ %s ] - Measured more Logs than allowed buffer size.\n", getName());
      }
   }
   else  rt_fprintf(stderr, "[ WARNING ][ %s ] - Exec #%ld not logged as timestamp was not set yet.\n", getName());

}

RTIME TaskDataLogger::logExec( )
{
   RTIME _logTime = rt_timer_read();
   if (execLogs[cptExecutions & MASK_SIZE].timestamp)
   {
      execLogs[cptExecutions & MASK_SIZE].duration = _logTime - execLogs[cptExecutions & MASK_SIZE].timestamp;

      if(execLogs[cptExecutions & MASK_SIZE].duration > deadline )
      {
         #if VERBOSE_DEBUG
         rt_fprintf(stderr, "[ Warning ] [ %s ] - Executed in %.2f ms.\n",getName(),execLogs[cptExecutions & MASK_SIZE].duration/1e6);
         #endif
         cptOutOfDeadline++;
      }else{
         #if VERBOSE_ASK
         //fprintf(stderr, "[ \033[1;32mPERFECT\033[0m ] Task : \033[1;32m%s\033[0m - \033[1;36m%.2f ms\033[0m\n",getName(),execLogs[cptExecutions].duration/1e6);
         #endif
      }
      cptExecutions++;
      if (cptExecutions == BUFF_SIZE)
      {
         rt_fprintf(stderr, "[ WARNING ][ %s ] - Measured more Logs than allowed buffer size.\n", getName());
      }
   }
   else  rt_fprintf(stderr, "[ WARNING ][ %s ] - Exec #%ld not logged as timestamp was not set yet.\n", getName());
   return _logTime;
}

void ChainDataLogger::logChain(timeLog _execLog)
{
   // _execLog.duration est un end timestamp en fait,
   // il faut faire la soustraction pour avoir la duree
   execLogs[cptExecutions & MASK_SIZE].timestamp = _execLog.timestamp;
   execLogs[cptExecutions & MASK_SIZE].duration = _execLog.duration;

      if(execLogs[cptExecutions & MASK_SIZE].duration > deadline )
      {
         #if VERBOSE_DEBUG
         rt_fprintf(stderr, "[ Warning ] [ %s ] - Executed in %.2f ms.\n",getName(),execLogs[cptExecutions & MASK_SIZE].duration/1e6);
         #endif
         cptOutOfDeadline++;
      }else{
         #if VERBOSE_ASK
         //fprintf(stderr, "[ \033[1;32mPERFECT\033[0m ] Task : \033[1;32m%s\033[0m - \033[1;36m%.2f ms\033[0m\n",getName(),execLogs[cptExecutions].duration/1e6);
         #endif
      }
      cptExecutions++;
      if (cptExecutions == BUFF_SIZE)
      {
         rt_fprintf(stderr, "[ WARNING ][ %s ] - Measured more Logs than allowed buffer size.\n", getName());
      }
}

void ChainDataLogger::setLogArray(int size)
{
   chainSize = size;
   for (auto& logElement : execLogs)
   {
      logElement.rWCETs = new unsigned long[chainSize];
   }
}

void ChainDataLogger::logWCET(unsigned long wcet)
{
   execLogs[cptExecutions & MASK_SIZE].rWCETs[cptWCET] = wcet;
   cptWCET++;
   if (cptWCET == chainSize) cptWCET = 0;

}

void TaskDataLogger::saveData(int nameSize, RT_TASK_INFO* cti)
{
   cerr << "Saving Task " << getName() << " data." << endl;
   std::ofstream outputFileTasksData;
   outputFileTasksData.open (outputFileName + TASKS_FILE, std::ios::app);    // TO APPEND :  //,ios_base::app);

   RTIME average_runtime = 0;
   RTIME max_runtime = 0;
   RTIME min_runtime = execLogs[0].duration;
   RTIME somme = 0;

   for (int i = LOG_VALUES_REMOVAL; i < (BUFF_SIZE - LOG_VALUES_REMOVAL ); ++i)
   {
      RTIME _dur = execLogs[i].duration;
      if (execLogs[i].timestamp != 0)
      {
         outputFileTasksData << std::setw(15) << execLogs[i].timestamp << " ; "
                              << std::setw(nameSize) << getName()         << " ; "
                              << std::setw(2) << taskInfos.fP.id         << " ; "
                              << std::setw(3) << taskInfos.fP.isHRT      << " ; "
                              << std::setw(4) << taskInfos.rtP.priority  << " ; "
                              << std::setw(10) << deadline                << " ; "
                              << std::setw(4) << taskInfos.rtP.affinity  << " ; "
                              << std::setw(10) << _dur                    << "\n";

         somme += _dur;
         if (_dur < min_runtime) min_runtime = _dur;
         if (_dur > max_runtime) max_runtime = _dur;
       }
   }

   outputFileTasksData.close();

   if (cptExecutions <= BUFF_SIZE && cptExecutions > 0)
      average_runtime = somme / (cptExecutions - LOG_VALUES_REMOVAL);
   else average_runtime = somme / BUFF_SIZE;

      int ret = 0;
      if (cti == NULL)
         ret = rt_task_inquire(NULL, cti);
      if (!ret)
      {
         std::ofstream outputFileResume;
         outputFileResume.open (outputFileName + RESUME_FILE, std::ios::app);    // TO APPEND :  //,ios_base::app);

         outputFileResume
            << "\nRunning summary for task "
            << getName() << ". (" << cti->pid << ", " << cti->prio << ", " << cti->name << ") Deadline : " << deadline / 1.0e6     << " ms.\n"
            << "Missed"     << " | "                 << "(2) | " << "Executions " << "\n"
            << std::setw(6) <<  cptOutOfDeadline << " | " << std::setw(3) << overruns << " | " << cptExecutions << " task times" << "\n"
            << "Primary Mode execution time - " << cti->stat.xtime/1.0e6 << " ms. Timeouts : " << cti->stat.timeout << "\n"
            <<         "  MIN  "   << " | " <<        "  AVG  "        << " | " <<      "  MAX"        << "\n"
            << min_runtime / 1.0e6 << " | " << average_runtime / 1.0e6 << " | " << max_runtime / 1.0e6 << " (ms) [task profile]" << "\n"
            << "   Mode Switches - " << cti->stat.msw << "\n"
            << "Context Switches - " << cti->stat.csw << "\n"
            << "Cobalt Sys calls - " << cti->stat.xsc
         << endl;

         outputFileResume.close();
      } else rt_fprintf(stderr, "[ ERROR ] - Error inquiring task %s : %s (%d).\n", getName(), getErrorName(ret), ret);

}

void ChainDataLogger::saveData(int nameSize, RT_TASK_INFO* cti )
{
   cerr << "Saving Chain " << getName() << " data. (" << LOG_VALUES_REMOVAL << ")" << endl;

   std::ofstream outputFileChainData;
   outputFileChainData.open (outputFileName + CHAIN_FILE, std::ios::app);    // TO APPEND :  //,ios_base::app);

   float average_runtime = 0;
   RTIME max_runtime = 0;
   RTIME min_runtime = execLogs[LOG_VALUES_REMOVAL].duration; // offset = LOG_VALUES_REMOVAL
   RTIME sommeTime = 0;
   for (int i = LOG_VALUES_REMOVAL; i < (BUFF_SIZE - LOG_VALUES_REMOVAL); ++i) // Last element probably not finished !
   {
      const RTIME _dur = execLogs[i].duration;
      if (execLogs[i].timestamp != 0)
      {
         outputFileChainData << std::setw(15)      << execLogs[i].timestamp << " ; "
         << std::setw(nameSize) << getName()        << " ; "
         << std::setw(2)       << chainInfos.taskChainID  << " ; "
         << std::setw(10)      << chainInfos.deadline    << " ; "
         << std::setw(10)      << _dur        << "\n";

         sommeTime += _dur;
         if (_dur < min_runtime) min_runtime = _dur;
         if (_dur > max_runtime) max_runtime = _dur;
      }
   }
   outputFileChainData.close();
/* CODE EN CAS D'OVERFLOW SUR LES CALCULS DE DURATION... A priori pas utile.
   RTIME s[16] = {0};
   int div = 0;
   for (int i = 0; i < BUFF_SIZE; ++i)
   {
      RTIME d = execLogs[i].duration;
      if (!d)
      {
         div = i;
         break;
      } else
      {
            for (int j = 0; j < 16; ++j)
            { // On gère 10 sous-moyennes possibles.
               if (  i >=  j   * BUFF_SIZE/16
                  && i < (j+1) * BUFF_SIZE/16)  s[j] += d;
            }

            if (d < min_runtime) min_runtime = d;
            if (d > max_runtime) max_runtime = d;
      }
   }
   int j = 0;
   for (j = 0; j < 16; ++j)
   {
      if (j <= (div / BUFF_SIZE))
         average_runtime += s[j] / (BUFF_SIZE/16); // Moyenne sur 1/16 des valeurs.
      else if ( j == ((div / BUFF_SIZE) + 1) )
         average_runtime += s[j] / (div % BUFF_SIZE);
      else break;
   }
*/

   if (cptExecutions <= BUFF_SIZE && cptExecutions > 0)
      average_runtime = sommeTime / (cptExecutions - LOG_VALUES_REMOVAL);
   else average_runtime = sommeTime / BUFF_SIZE; // on a pas enregistré plus de BUFF_SIZE valeurs;

   std::ofstream outputFileResume;
   outputFileResume.open (outputFileName + RESUME_FILE, std::ios::app);    // TO APPEND :  //,ios_base::app);

   #if VERBOSE_INFO
   outputFileResume
      << "\n[CHAIN] Summary for " << getName() << ". (id = " << chainInfos.taskChainID << " )"
      << " - Deadline : " << deadline / 1.0e6     << " ms." << "\n"
      << "Chain loops" << " | " << "Anticipated Misses" << " | "        <<   "Missed"   <<  "\n"
      << std::setw(11) << cptExecutions         << " | "
      << std::setw(18) <<  cptAnticipatedMisses << " | "
      << std::setw(6) << cptOutOfDeadline       << "\n"

      <<         "  MIN  "   << " | " <<        "  AVG  "        << " | " <<      "  MAX"        << "\n"
      << std::setw(7) << min_runtime / 1.0e6 << " | " << average_runtime / 1.0e6 << " | " << max_runtime / 1.0e6 << " runtimes (ms)"
      << endl;
   #endif

   outputFileResume.close();
}


char* ChainDataLogger::getName()
{
   return chainInfos.name;
}


char* TaskDataLogger::getName()
{
   return taskInfos.fP.name;
}

#include <iomanip>
#include "dataLogger.h"
#include "NanoLog.h"

DataLogger::DataLogger(string expeName){

   outputFileName = expeName;
   cptOutOfDeadline = 0;
   cptExecutions = 0;
   execLogs = {0};
   overruns = 0;
   //nanolog::initialize(nanolog::GuaranteedLogger(), ".", "nanolog", 1);
   //LOG_INFO << "This is a Information Log test !";
   //LOG_WARN << "This is a WARNING log.";
   //LOG_CRIT << "This is a CRITical log message.";
}

ChainDataLogger::ChainDataLogger(end2endDeadlineStruct _chainInfos, string expeName) : DataLogger(expeName)
{
   chainInfos = _chainInfos;
   deadline = _chainInfos.deadline;
   cptAnticipatedMisses = 0;
}

TaskDataLogger::TaskDataLogger(rtTaskInfosStruct _taskInfos, string expeName) : DataLogger(expeName)
{
   taskInfos = _taskInfos;
   deadline = _taskInfos.rtP.periodicity;
}

void DataLogger::logStart(RTIME startTime)
{
   execLogs[cptExecutions].timestamp = startTime;
}


RTIME DataLogger::logStart()
{
   return (execLogs[cptExecutions].timestamp = rt_timer_read());
}

void DataLogger::logExec(RTIME endTime)
{
   if (execLogs[cptExecutions].timestamp)
   {
      execLogs[cptExecutions].duration = endTime - execLogs[cptExecutions].timestamp;

      if(execLogs[cptExecutions].duration > deadline )
      {
         #if VERBOSE_ASK
         rt_fprintf(stderr, "[ Warning ] [ %s ] - Executed in %.2f ms.\n",getName(),execLogs[cptExecutions].duration/1e6);
         #endif
         cptOutOfDeadline++;
      }else{
         #if VERBOSE_ASK
         //fprintf(stderr, "[ \033[1;32mPERFECT\033[0m ] Task : \033[1;32m%s\033[0m - \033[1;36m%.2f ms\033[0m\n",getName(),execLogs[cptExecutions].duration/1e6);
         #endif
      }
      cptExecutions++;
      //return execLogs[cptExecutions - 1].duration;
      if (cptExecutions > 4095)
      {
         cptExecutions = 0;
         rt_fprintf(stderr, "[ WARNING ][ %s ] - Measured more Logs than allowed buffer size.\n", getName());
      }
   }
   else  rt_fprintf(stderr, "[ WARNING ][ %s ] - Exec #%ld not logged as timestamp was not set yet.\n", getName());

}

RTIME DataLogger::logExec( )
{
   RTIME _logTime = rt_timer_read();
   if (execLogs[cptExecutions].timestamp)
   {
      execLogs[cptExecutions].duration = _logTime - execLogs[cptExecutions].timestamp;

      if(execLogs[cptExecutions].duration > deadline )
      {
         #if VERBOSE_ASK
         rt_fprintf(stderr, "[ Warning ] [ %s ] - Executed in %.2f ms.\n",getName(),execLogs[cptExecutions].duration/1e6);
         #endif
         cptOutOfDeadline++;
      }else{
         #if VERBOSE_ASK
         //fprintf(stderr, "[ \033[1;32mPERFECT\033[0m ] Task : \033[1;32m%s\033[0m - \033[1;36m%.2f ms\033[0m\n",getName(),execLogs[cptExecutions].duration/1e6);
         #endif
      }
      cptExecutions++;
      if (cptExecutions > 4095)
      {
         cptExecutions = 0;
         rt_fprintf(stderr, "[ WARNING ][ %s ] - Measured more Logs than allowed buffer size.\n", getName());
      }
   }
   else  rt_fprintf(stderr, "[ WARNING ][ %s ] - Exec #%ld not logged as timestamp was not set yet.\n", getName());
   return _logTime;
}

void TaskDataLogger::saveData(int nameSize, RT_TASK_INFO* cti)
{
   std::ofstream outputFileTasksData;
   outputFileTasksData.open (outputFileName + TASKS_FILE, std::ios::app);    // TO APPEND :  //,ios_base::app);

   RTIME average_runtime = 0;
   RTIME max_runtime = 0;
   RTIME min_runtime = execLogs[0].duration;
   RTIME somme = 0;

   if (cptExecutions > 0)
   for (int i = 0; i < cptExecutions; ++i)
   {
      RTIME _dur = execLogs[i].duration;

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
   else cerr << "[" << getName() << "] -" << "Error : no logs to print !" << endl;

   outputFileTasksData.close();
   if (cptExecutions > 0)
   {

      average_runtime = somme / cptExecutions;

      int ret = 0;
      if (cti == NULL)
         ret = rt_task_inquire(NULL, cti);
      if (!ret)
      {
         std::ofstream outputFileResume;
         outputFileResume.open (outputFileName + RESUME_FILE, std::ios::app);    // TO APPEND :  //,ios_base::app);

         outputFileResume
            << "\nRunning summary for task "
            << getName() << ". (" << cti->pid << ", " << cti->prio << ", " << cti->name << ")" << "\n"
            << "Deadline : " << deadline / 1.0e6     << " ms.  Missed"     << " | "                 << "(2) | " << "Executions " << "\n"
            << "                    " << std::setw(6) <<  cptOutOfDeadline << " | " << std::setw(3) << overruns << " | " << cptExecutions << " times" << "\n"
            << "Primary Mode execution time - " << cti->stat.xtime/1.0e6 << " ms. Timeouts : " << cti->stat.timeout << "\n"
            <<         "  MIN  "   << " | " <<        "  AVG  "        << " | " <<      "  MAX"        << "\n"
            << min_runtime / 1.0e6 << " | " << average_runtime / 1.0e6 << " | " << max_runtime / 1.0e6 << " (ms)" << "\n"
            << "   Mode Switches - " << cti->stat.msw << "\n"
            << "Context Switches - " << cti->stat.csw << "\n"
            << "Cobalt Sys calls - " << cti->stat.xsc
         << endl;

         outputFileResume.close();
      } else rt_fprintf(stderr, "Error inquiring task %s : %s (%d).\n", getName(), getErrorName(ret), ret);
   }


}

void ChainDataLogger::saveData(int nameSize, RT_TASK_INFO* cti )
{
   cerr << "Saving Chain " << getName() << " data." << endl;
   std::ofstream outputFileChainData;
   outputFileChainData.open (outputFileName + CHAIN_FILE, std::ios::app);    // TO APPEND :  //,ios_base::app);

   double average_runtime = 0;
   RTIME max_runtime = 0;
   RTIME min_runtime = execLogs[0].duration;
   double sommeTime = 0;

   if (cptExecutions <= 0) cerr << "[ " << getName() << " ] - " << "Error : no logs to print !" << endl;
   else for (int i = 0; i < cptExecutions; i++)
   {
      const RTIME _dur = execLogs[i].duration;

      outputFileChainData << std::setw(15)      << execLogs[i].timestamp << " ; "
                          << std::setw(nameSize) << getName()        << " ; "
                          << std::setw(2)       << chainInfos.taskChainID  << " ; "
                          << std::setw(10)      << chainInfos.deadline    << " ; "
                          << std::setw(10)      << _dur        << "\n";

      sommeTime += _dur;
      if (_dur < min_runtime) min_runtime = _dur;
      if (_dur > max_runtime) max_runtime = _dur;
   }
   outputFileChainData.close();

   if (cptExecutions > 0)
   {
      std::ofstream outputFileResume;
      outputFileResume.open (outputFileName + RESUME_FILE, std::ios::app);    // TO APPEND :  //,ios_base::app);

      average_runtime = sommeTime / cptExecutions;
      #if VERBOSE_INFO
      outputFileResume
         << "\n[CHAIN] Summary for " << getName() << ". (id = " << chainInfos.taskChainID << " )" << "\n"
         << "Deadline : " << deadline / 1.0e6     << " ms."
         << "Chain loops" << " Anticipated Misses" << " | "        <<   "Missed"     << " | " <<  "\n"
         << std::setw(11) << cptExecutions         << " | "
         << std::setw(19) <<  cptAnticipatedMisses << " | "
         << std::setw(6) << cptOutOfDeadline       << "\n"

         <<         "  MIN  "   << " | " <<        "  AVG  "        << " | " <<      "  MAX"        << "\n"
         << std::setw(7) << min_runtime / 1.0e6 << " | " << average_runtime / 1.0e6 << " | " << max_runtime / 1.0e6 << " runtimes (ms)"
         << endl;
      #endif

      outputFileResume.close();
   }
}

char* ChainDataLogger::getName()
{
   return chainInfos.name;
}


char* TaskDataLogger::getName()
{
   return taskInfos.fP.name;
}

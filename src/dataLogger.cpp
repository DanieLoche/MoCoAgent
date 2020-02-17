#include "dataLogger.h"
#include <iomanip>

DataLogger::DataLogger(){}

ChainDataLogger::ChainDataLogger(end2endDeadlineStruct* chainInfos)
{
   deadline = chainInfos->deadline;

   cptAnticipatedMisses = 0;
   cptOutOfDeadline = 0;
   cptExecutions = 0;
   execLogs = {0};
 //cout << "Init of Chains logger is okay." << endl;
}

TaskDataLogger::TaskDataLogger(rtTaskInfosStruct* _taskInfos)
{
   taskInfos = _taskInfos;
  deadline = taskInfos->rtP.periodicity;

  cptOutOfDeadline = 0;
  cptExecutions = 0;
  execLogs = {0};
//cout << "Init of task logger for task " << getName() << " is okay." << endl;

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
        fprintf(stderr, "[  \033[1;31mERROR\033[0m  ] Task : \033[1;31m%s\033[0m - \033[1;36m%.2f ms\033[0m\n",getName(),execLogs[cptExecutions].duration/1e6);
        #endif
        cptOutOfDeadline++;
      }else{
        #if VERBOSE_ASK
          fprintf(stderr, "[ \033[1;32mPERFECT\033[0m ] Task : \033[1;32m%s\033[0m - \033[1;36m%.2f ms\033[0m\n",getName(),execLogs[cptExecutions].duration/1e6);
        #endif
      }
      cptExecutions++;
      //return execLogs[cptExecutions - 1].duration;
      if (cptExecutions > 4095)
      {
         cptExecutions = 0;
         cerr << "[" << getName() << "] " << "WARNING - Measured more than allowed buffer size." << endl;
      }
   }
   else cerr << "[" << getName() << "] " << "- WARNING : Exec not logged as timestamp was not set yet." << endl;

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
       fprintf(stderr, "[  \033[1;31mERROR\033[0m  ] Task : \033[1;31m%s\033[0m - \033[1;36m%.2f ms\033[0m\n",getName(),execLogs[cptExecutions].duration/1e6);
       #endif
       cptOutOfDeadline++;
      }else{
       #if VERBOSE_ASK
       fprintf(stderr, "[ \033[1;32mPERFECT\033[0m ] Task : \033[1;32m%s\033[0m - \033[1;36m%.2f ms\033[0m\n",getName(),execLogs[cptExecutions].duration/1e6);
       #endif
      }
      cptExecutions++;
      if (cptExecutions > 4095)
      {
        cptExecutions = 0;
        cerr << "[" << getName() << "] " << "WARNING - Measured more than allowed buffer size." << endl;
      }
   }
   else cerr << "[" << getName() << "] " << "- WARNING : Exec not logged as timestamp was not set yet." << endl;
   return _logTime;
}

void TaskDataLogger::saveData(string file, int nameSize)
{
   std::ofstream outputFileTasksData;
   outputFileTasksData.open (file + "_Expe.csv", std::ios::app);    // TO APPEND :  //,ios_base::app);

   RTIME average_runtime = 0;
   RTIME max_runtime = 0;
   RTIME min_runtime = 1.e9;
   double somme = 0;

   if (!(cptExecutions > 0)) cerr << "[" << getName() << "] -" << "Error : no logs to print !" << endl;
   else for (int i = 0; i < cptExecutions; i++)
   {
    RTIME _dur = execLogs[i].duration;

    outputFileTasksData << std::setw(15) << execLogs[i].timestamp << " ; "
                   << std::setw(nameSize) << getName()         << " ; "
                   << std::setw(2) << taskInfos->fP.id         << " ; "
                   << std::setw(3) << taskInfos->fP.isHRT      << " ; "
                   << std::setw(4) << taskInfos->rtP.priority  << " ; "
                   << std::setw(10) << deadline                << " ; "
                   << std::setw(4) << taskInfos->rtP.affinity  << " ; "
                   << std::setw(10) << _dur                    << "\n";

    somme += _dur;
    if (_dur < min_runtime) min_runtime = _dur;
    if (_dur > max_runtime) max_runtime = _dur;
   }
   outputFileTasksData.close();

   #if VERBOSE_INFO
   std::ofstream outputFileResume;
   outputFileResume.open (file + "_Resume.txt", std::ios::app);    // TO APPEND :  //,ios_base::app);

   average_runtime = somme / cptExecutions;
   RT_TASK _t;
   ERROR_MNG(rt_task_bind(&_t, getName(), TM_INFINITE));
   RT_TASK_INFO cti;
   rt_task_inquire(&_t, &cti);

   outputFileResume << "\nRunning summary for task " << getName() << ". (" << cti.pid << ", " << cti.prio << ", " << cti.name << ")" << "\n"
      << "Deadline : " << deadline / 1.0e6     << " ms.  Missed"     << " | "                 << "(2) | " << "Executions " << "\n"
      << "                    " << std::setw(6) <<  cptOutOfDeadline << " | " << std::setw(3) << overruns << " | " << cptExecutions << " times" << "\n"
      << "Primary Mode execution time - " << cti.stat.xtime/1.0e6 << " ms. Timeouts : " << cti.stat.timeout << "\n"
      <<         "  MIN  "   << " | " <<        "  AVG  "        << " | " <<      "  MAX"        << "\n"
      << min_runtime / 1.0e6 << " | " << average_runtime / 1.0e6 << " | " << max_runtime / 1.0e6 << " (ms)" << "\n"
      << "   Mode Switches - " << cti.stat.msw << "\n"
      << "Context Switches - " << cti.stat.csw << "\n"
      << "Cobalt Sys calls - " << cti.stat.xsc
      << endl;

   outputFileResume.close();
   #endif

}

void ChainDataLogger::saveData(string file, int nameSize)
{
   std::ofstream outputFileChainData;
   outputFileChainData.open (file + "_Chains.csv", std::ios::app);    // TO APPEND :  //,ios_base::app);

   double average_runtime = 0;
   RTIME max_runtime = 0;
   RTIME min_runtime = 1.e9;
   double sommeTime = 0;
   outputFileChainData << std::setw(15)      << "timestamp" << " ; "
                  << std::setw(strlen(getName())) << "Chain"     << " ; "
                  << std::setw(2)            << "ID"        << " ; "
                  << std::setw(10)           << "deadline"  << " ; "
                  << std::setw(10)           << "duration"  << endl;

   if (cptExecutions <= 0) cerr << "[" << getName() << "] -" << "Error : no logs to print !" << endl;
   else for (int i = 0; i < cptExecutions; i++)
   {
      RTIME _dur = execLogs[i].duration;

      outputFileChainData << std::setw(15) << execLogs[i].timestamp << " ; "
                     << std::setw(strlen(getName())) << getName()        << " ; "
                     << std::setw(2)         << chainInfos->taskChainID  << " ; "
                     << std::setw(10)        << deadline    << " ; "
                     << std::setw(10)        << _dur        << "\n";

      sommeTime += _dur;
      if (_dur < min_runtime) min_runtime = _dur;
      if (_dur > max_runtime) max_runtime = _dur;
   }
  outputFileChainData.close();

  std::ofstream outputFileResume;
  outputFileResume.open (file + "_Resume.txt", std::ios::app);    // TO APPEND :  //,ios_base::app);

  average_runtime = sommeTime / cptExecutions;
  #if VERBOSE_INFO
  outputFileResume << "\nRunning summary for Chain " << getName() << ". ( " << chainInfos->taskChainID << " )" << "\n"
                   << "Deadline : " << deadline / 1.0e6     << " ms." << " Anticipated Misses" << " | "
                   <<   " Missed "     << " | " << " Chain loops " << "\n"

                   << "                  "         << std::setw(20) <<  cptAnticipatedMisses << " | "
                   << std::setw(10) << cptOutOfDeadline << " | " << cptExecutions << " times" << "\n"

                   <<         "  MIN  "   << " | " <<        "  AVG  "        << " | " <<      "  MAX"        << "\n"
                   << std::setw(7) << min_runtime / 1.0e6 << " | " << average_runtime / 1.0e6 << " | " << max_runtime / 1.0e6 << " runtimes (ms)"
                   << endl;
  #endif

  outputFileResume.close();
}

char* ChainDataLogger::getName()
{
   return chainInfos->name;
}


char* TaskDataLogger::getName()
{
   return taskInfos->fP.name;
}

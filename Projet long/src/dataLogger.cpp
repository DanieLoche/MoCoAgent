#include "tools.h"
#include "dataLogger.h"

#include <iomanip>


DataLogger::DataLogger(){}

ChainDataLogger::ChainDataLogger(end2endDeadlineStruct* chainInfos)
{
  strcpy(name, chainInfos->name);
  id = chainInfos->taskChainID;
  deadline = chainInfos->deadline;

  cptAnticipatedMisses = 0;
  cptOutOfDeadline = 0;
  cptExecutions = 0;
  execLogs = {0};
 //cout << "Init of Chains logger is okay." << endl;
}

TaskDataLogger::TaskDataLogger(rtTaskInfosStruct* taskInfos)
{
  task = taskInfos->task;
  strcpy(name, taskInfos->name);
  id = taskInfos->id;
  isHardRealTime = taskInfos->isHardRealTime;
  affinity = taskInfos->affinity;
  deadline = taskInfos->periodicity;

  cptOutOfDeadline = 0;
  cptExecutions = 0;
  execLogs = {0};
//cout << "Init of task logger for task " << name << " is okay." << endl;

}

void DataLogger::logStart(timespec startTime)
{
  execLogs[cptExecutions].timestamp = startTime;
}


timespec DataLogger::logStart()
{
   getTime( &execLogs[cptExecutions].timestamp );
  return execLogs[cptExecutions].timestamp;
}

void DataLogger::logExec(timespec endTime)
{
   if (execLogs[cptExecutions].timestamp.tv_sec > 0)
   {
      execLogs[cptExecutions].duration = diffTime(endTime, execLogs[cptExecutions].timestamp);

      if(getTime_ms(execLogs[cptExecutions].duration) > deadline )
      {
         cptOutOfDeadline++;
        #if VERBOSE_ASK
        rt_printf("[  \033[1;31mERROR\033[0m  ] Task : \033[1;31m%s\033[0m - \033[1;36m%.2f ms\033[0m\n",name,execLogs[cptExecutions].duration/1e6);
        #endif
      }else{
        #if VERBOSE_ASK
          rt_printf("[ \033[1;32mPERFECT\033[0m ] Task : \033[1;32m%s\033[0m - \033[1;36m%.2f ms\033[0m\n",name,execLogs[cptExecutions].duration/1e6);
        #endif
      }
      cptExecutions++;
      //return execLogs[cptExecutions - 1].duration;
   }
   else cout << "Warning : Exec not logged because timestamp was not set yet." << endl;

}

timespec DataLogger::logExec( )
{
  timespec _logTime;
  getTime(&_logTime);
  execLogs[cptExecutions].duration = diffTime(_logTime, execLogs[cptExecutions].timestamp);

  if( getTime_ms(execLogs[cptExecutions].duration) > deadline )
  {
    #if VERBOSE_ASK
    rt_printf("[  \033[1;31mERROR\033[0m  ] Task : \033[1;31m%s\033[0m - \033[1;36m%.2f ms\033[0m\n",name,execLogs[cptExecutions].duration/1e6);
    #endif
    cptOutOfDeadline++;
  }else{
    #if VERBOSE_ASK
      rt_printf("[ \033[1;32mPERFECT\033[0m ] Task : \033[1;32m%s\033[0m - \033[1;36m%.2f ms\033[0m\n",name,execLogs[cptExecutions].duration/1e6);
    #endif
  }
  cptExecutions++;
  return _logTime;
}

void TaskDataLogger::saveData(string file, int nameSize)
{
  std::ofstream outputFileTasksData;
  string outputFileName = file + "_Expe.csv";
  outputFileTasksData.open (outputFileName, std::ios::app);    // TO APPEND :  //,ios_base::app);

  double average_runtime = 0;
  uint64_t max_runtime = 0;
  uint64_t min_runtime = 1.e9;
  uint64_t somme = 0;






  if (!(cptExecutions > 0)) cout << "Error : no logs to print !" << endl;
  else for (int i = 0; i < cptExecutions; i++)
  {
    uint64_t _dur = getTime_us(execLogs[i].duration);

    outputFileTasksData << std::setw(15) << getTime_us(execLogs[i].timestamp) << " ; "
                   << std::setw(nameSize) << name           << " ; "
                   << std::setw(2) << id                    << " ; "
                   << std::setw(3) << isHardRealTime        << " ; "
                   << std::setw(10) << deadline             << " ; "
                   << std::setw(4) << affinity              << " ; "
                   << std::setw(10) << _dur                 << "\n";

    somme += _dur;
    if (_dur < min_runtime) min_runtime = _dur;
    if (_dur > max_runtime) max_runtime = _dur;
  }
  outputFileTasksData.close();

  #if VERBOSE_INFO
  std::ofstream outputFileResume;
  outputFileName = file + "_Resume.txt";
  outputFileResume.open (outputFileName, std::ios::app);    // TO APPEND :  //,ios_base::app);

  average_runtime = somme / cptExecutions;
  if (task != NULL)
  {
    RT_TASK_INFO cti;
    rt_task_inquire(task, &cti);

    outputFileResume << "\nRunning summary for task " << name << ". (" << cti.pid << ", " << cti.prio << ", " << cti.name << ")" << "\n"
         << "Deadline : " << deadline / 1.0e6     << " ms.  Missed"     << " | " << "Executions " << "\n"
         << "                    " << std::setw(6) <<  cptOutOfDeadline << " | " << cptExecutions << " times" << "\n"
         << "Primary Mode execution time - " << cti.stat.xtime/1.0e6 << " ms. Timeouts : " << cti.stat.timeout << "\n"
         <<         "  MIN  "   << " | " <<        "  AVG  "        << " | " <<      "  MAX"        << "\n"
         << min_runtime / 1.0e6 << " | " << average_runtime / 1.0e6 << " | " << max_runtime / 1.0e6 << " (ms)" << "\n"
         << "   Mode Switches - " << cti.stat.msw << "\n"
         << "Context Switches - " << cti.stat.csw << "\n"
         << "Cobalt Sys calls - " << cti.stat.xsc
         << endl;
    }

    outputFileResume.close();
    #endif

}

void ChainDataLogger::saveData(string file, int nameSize)
{
   std::ofstream outputFileChainData;
   string outputFileName = file + "_Chains.csv";
   outputFileChainData.open (outputFileName, std::ios::app);    // TO APPEND :  //,ios_base::app);

   double average_runtime = 0;
   uint64_t max_runtime = 0;
   uint64_t min_runtime = 1.e9;
   uint64_t sommeTime = 0;
   outputFileChainData << std::setw(15)      << "timestamp" << " ; "
                  << std::setw(strlen(name)) << "Chain"     << " ; "
                  << std::setw(2)            << "ID"        << " ; "
                  << std::setw(10)           << "deadline"  << " ; "
                  << std::setw(10)           << "duration"  << endl;

   if (!(cptExecutions > 0)) cout << "Error : no logs to print !" << endl;
   else for (int i = 0; i < cptExecutions; i++)
   {
      uint64_t _dur = getTime_us(execLogs[i].duration);

      outputFileChainData << std::setw(15) << getTime_us(execLogs[i].timestamp) << " ; "
                     << std::setw(strlen(name)) << name        << " ; "
                     << std::setw(2)            << id          << " ; "
                     << std::setw(10)           << deadline    << " ; "
                     << std::setw(10)           << _dur        << "\n";



      sommeTime += _dur;
      if (_dur < min_runtime) min_runtime = _dur;
      if (_dur > max_runtime) max_runtime = _dur;
   }
  outputFileChainData.close();

  std::ofstream outputFileResume;
  outputFileName = file + "_Resume.txt";
  outputFileResume.open (outputFileName, std::ios::app);    // TO APPEND :  //,ios_base::app);

  average_runtime = sommeTime / cptExecutions;
  #if VERBOSE_INFO
  outputFileResume << "\nRunning summary for Chain " << name << ". ( " << id << " )" << "\n"
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

char* DataLogger::getName()
{
   return name;
}

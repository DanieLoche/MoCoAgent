#include "tools.h"
#include "dataLogger.h"

#include <iomanip>


DataLogger::DataLogger(){}

ChainDataLogger::ChainDataLogger(end2endDeadlineStruct* chainInfos)
{
  strcpy(name, chainInfos->name);
  id = chainInfos->taskChainID;
  affinity = 0;
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
        rt_printf("[  \033[1;31mERROR\033[0m  ] Task : \033[1;31m%s\033[0m - \033[1;36m%.2f ms\033[0m\n",name,execLogs[cptExecutions].duration/1e6);
        #endif
        cptOutOfDeadline++;
      }else{
        #if VERBOSE_ASK
          rt_printf("[ \033[1;32mPERFECT\033[0m ] Task : \033[1;32m%s\033[0m - \033[1;36m%.2f ms\033[0m\n",name,execLogs[cptExecutions].duration/1e6);
        #endif
      }
      cptExecutions++;
      //return execLogs[cptExecutions - 1].duration;
   }
   else cout << "Warning : Exec not logged as timestamp was not set yet." << endl;

}

RTIME DataLogger::logExec( )
{
  RTIME _logTime = rt_timer_read();
  execLogs[cptExecutions].duration = _logTime - execLogs[cptExecutions].timestamp;

  if(execLogs[cptExecutions].duration > deadline )
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
  std::ofstream myFile;
  myFile.open (file, std::ios::app);    // TO APPEND :  //,ios_base::app);

  //myFile << "timestamp ; name ; ID ; HRT ; deadline ; duration ; affinity \n";
  RTIME average_runtime = 0;
  RTIME max_runtime = 0;
  RTIME min_runtime = 1.e9;
  double somme = 0;
  for (int i = 0; i < cptExecutions; i++)
  {
    RTIME _dur = execLogs[i].duration;

    myFile << std::setw(15) << execLogs[i].timestamp << " ; "
           << std::setw(nameSize) << name           << " ; "
           << std::setw(2) << id             << " ; "
           << std::setw(3) << isHardRealTime << " ; "
           << std::setw(10) << deadline       << " ; "
           << std::setw(4) << affinity       << " ; "
           << std::setw(10) << _dur           << "\n";

    somme += _dur;
    if (_dur < min_runtime) min_runtime = _dur;
    if (_dur > max_runtime) max_runtime = _dur;
  }
  myFile.close();

  average_runtime = somme / cptExecutions;
  #if VERBOSE_INFO
  if (task != NULL)
  {
    RT_TASK_INFO cti;
    rt_task_inquire(task, &cti);

    cout << "\nRunning summary for task " << name << ". (" << cti.pid << ", " << cti.prio << ", " << cti.name << ")" << "\n"
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
#endif

}

void ChainDataLogger::saveData(string file, int nameSize)
{
   RTIME average_runtime = 0;
   RTIME max_runtime = 0;
   RTIME min_runtime = 9.e9;
   RTIME sommeTime = 0;

   std::ofstream myFile;
   myFile.open (file, std::ios::app);    // TO APPEND :  //,ios_base::app);

   myFile << std::setw(15) << "timestamp" << " ; "
          << std::setw(strlen(name)) << "Chain"     << " ; "
          << std::setw(2) << "ID"        << " ; "
          << std::setw(10) << "deadline"  << " ; "
          << std::setw(4)  << "aff."  << " ; "
          << std::setw(10) << "duration"  << "\n";

   for (int i = 0; i < cptExecutions; i++)
   {
      RTIME _dur = execLogs[i].duration;

      myFile << std::setw(15) << execLogs[i].timestamp << " ; "
             << std::setw(strlen(name)) << name                  << " ; "
             << std::setw(2)  << id                    << " ; "
             << std::setw(10) << deadline              << " ; "
             << std::setw(4)  << affinity              << " ; "
             << std::setw(10) << _dur                  << "\n";

      sommeTime += _dur;
      if (_dur < min_runtime) min_runtime = _dur;
      if (_dur > max_runtime) max_runtime = _dur;
   }

  average_runtime = sommeTime / cptExecutions;
  #if VERBOSE_INFO
  cout << "\nRunning summary for Chain " << name << ". ( " << id << " )" << "\n"
       << "Deadline : " << deadline / 1.0e6     << " ms." << " Anticipated Misses" << " | "
       <<   " Missed "     << " | " << " Chain loops " << "\n"

       << "                  "         << std::setw(20) <<  cptAnticipatedMisses << " | "
       << std::setw(10) << cptOutOfDeadline << " | " << cptExecutions << " times" << "\n"

       <<         "  MIN  "   << " | " <<        "  AVG  "        << " | " <<      "  MAX"        << "\n"
       << std::setw(7) << min_runtime / 1.0e6 << " | " << average_runtime / 1.0e6 << " | " << max_runtime / 1.0e6 << " runtimes (ms)"
       << endl;
  #endif


  myFile.close();

}

char* DataLogger::getName()
{
   return name;
}

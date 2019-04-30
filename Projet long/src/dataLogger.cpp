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
//cout << "Init of task logger for task " << name << " is okay." << endl;
}

TaskDataLogger::TaskDataLogger(rtTaskInfosStruct* taskInfos)
{
  task = taskInfos->task;
  strcpy(name, taskInfos->name);
  id = taskInfos->id;
  isHardRealTime = taskInfos->isHardRealTime;
  affinity = taskInfos->affinity;
  deadline = taskInfos->deadline;

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

void TaskDataLogger::saveData(string file)
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

    myFile << execLogs[i].timestamp << " ; "
           << name           << " ; "
           << id             << " ; "
           << isHardRealTime << " ; "
           << deadline       << " ; "
           << affinity       << " ; "
           << _dur           << "\n";
    somme += _dur;
    if (_dur < min_runtime) min_runtime = _dur;
    else if (_dur > max_runtime) max_runtime = _dur;
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
         << min_runtime / 1.0e6 << " | " << average_runtime / 1.0e6 << " | " << max_runtime / 1.0e6 << " runtimes (ms)" << "\n"
         << "   Mode Switches - " << cti.stat.msw << "\n"
         << "Context Switches - " << cti.stat.csw << "\n"
         << "Cobalt Sys calls - " << cti.stat.xsc
         << endl;
    }
#endif

}

void ChainDataLogger::saveData(string file)
{
  std::ofstream myFile;
  myFile.open (file, std::ios::app);    // TO APPEND :  //,ios_base::app);

  //myFile << "timestamp ; name ; ID ; HRT ; deadline ; duration ; affinity \n";
  RTIME average_runtime = 0;
  RTIME max_runtime = 0;
  RTIME min_runtime = 1.e9;
  double somme = 0;

  myFile << "timestamp ; Chain ; ID ; deadline ; affinity ; duration \n";

  for (int i = 0; i < cptExecutions; i++)
  {
    RTIME _dur = execLogs[i].duration;

    myFile << execLogs[i].timestamp << " ; "
           << name           << " ; "
           << id             << " ; "
           << deadline       << " ; "
           << affinity       << " ; "
           << _dur           << "\n";
    somme += _dur;
    if (_dur < min_runtime) min_runtime = _dur;
    else if (_dur > max_runtime) max_runtime = _dur;
  }

  average_runtime = somme / cptExecutions;
  #if VERBOSE_INFO
  cout << "\nRunning summary for Chain " << name << ". ( " << id << " )" << "\n"
       << "Deadline : " << deadline / 1.0e6     << " ms." << " Anticipated Misses" << " | "
       <<   " Missed "     << " | " << " Chain loops " << "\n"

       << "                    "         << std::setw(10) <<  cptAnticipatedMisses << " | "
       << std::setw(10) << cptOutOfDeadline << " | " << cptExecutions << " times" << "\n"

       <<         "  MIN  "   << " | " <<        "  AVG  "        << " | " <<      "  MAX"        << "\n"
       << min_runtime / 1.0e6 << " | " << average_runtime / 1.0e6 << " | " << max_runtime / 1.0e6 << " runtimes (ms)"
       << endl;
  #endif


  myFile.close();

}

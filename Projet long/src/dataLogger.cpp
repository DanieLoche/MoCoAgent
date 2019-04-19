#include "dataLogger.h"

#include <iomanip>

DataLogger::DataLogger(end2endDeadlineStruct* chainInfos)
{
  task = NULL;
  strcpy(name, chainInfos->name);
  id = chainInfos->taskChainID;
  isHardRealTime = 0;
  affinity = 0;
  deadline = chainInfos->deadline;

  cptOutOfDeadline = 0;
  cptExecutions = 0;
  execLogs = {0};
//cout << "Init of task logger for task " << name << " is okay." << endl;

}

DataLogger::DataLogger(rtTaskInfosStruct* taskInfos)
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

RTIME DataLogger::logExec(RTIME endTime)
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
  return execLogs[cptExecutions - 1].duration;
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

void DataLogger::saveData(string file)
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

  average_runtime = somme / cptExecutions;
  RT_TASK_INFO cti;
  rt_task_inquire(task, &cti);

 #if VERBOSE_INFO
 cout << "\nRunning summary for task " << name << ". (" << cti.pid << ", " << cti.prio << ", " << cti.name << ")" << endl
      << "Deadline : " << deadline / 1.0e6 << " ms.  Missed |  Executions " << endl
      << "                    " << std::setw(6) <<  cptOutOfDeadline << " | " << cptExecutions << " times" << endl
      << "Primary Mode execution time - " << cti.stat.xtime/1.0e6 << " ms. Timeouts : " << cti.stat.timeout << endl
      << "  MIN  " << " | " << "  AVG  " << " | " << "  MAX" << endl
      << min_runtime / 1.0e6 << " | " << average_runtime / 1.0e6 << " | " << max_runtime / 1.0e6 << " runtimes (ms)" << endl
      << "   Mode Switches - " << cti.stat.msw << endl
      << "Context Switches - " << cti.stat.csw << endl
      << "Cobalt Sys calls - " << cti.stat.xsc << endl;
 #endif


  myFile.close();

}

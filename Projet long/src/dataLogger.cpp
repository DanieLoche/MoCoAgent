#include "dataLogger.h"

DataLogger::DataLogger(rtTaskInfosStruct* taskInfos)
{
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

void DataLogger::logStart()
{
  execLogs[cptExecutions].timestamp = rt_timer_read();
}

RTIME DataLogger::logExec( )
{
  execLogs[cptExecutions].duration = rt_timer_read() - execLogs[cptExecutions].timestamp;

  /*cout << " timestamp : " << execLogs[cptExecutions].timestamp  << endl
      << " duration : " << execLogs[cptExecutions].duration     << endl
      << " deadline : " << deadline             << endl
      << " Execution n_" << cptExecutions       << endl;
  usleep(500000);*/
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

 #if VERBOSE_INFO
 cout << "\nRunning summary for task " << name << ".\n"
      << "Average runtime : "      << average_runtime / 1.0e6 << " ms\n"
      << "Max runtime : "          << max_runtime / 1.0e6     << " ms\n"
      << "Min runtime : "          << min_runtime / 1.0e6     << " ms\n"
      << "Deadline :"              << deadline / 1.0e6        << " ms\n"
      << "Out of Deadline : "      << cptOutOfDeadline        << " times\n"
      << "Number of executions : " << cptExecutions << " times\n";
 #endif


  myFile.close();

}

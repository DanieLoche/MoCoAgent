#ifndef DATALOGGER_H
#define DATALOGGER_H

#include "tools.h"
#include <array>

struct timeLog
{
  RTIME timestamp;
  RTIME duration;
};

class DataLogger
{
  private :
    char name[32];
    int id;
    int  isHardRealTime;
    int affinity;
    RTIME deadline;

  public :
    DataLogger();
    DataLogger(rtTaskInfosStruct*);

    std::array<timeLog, 4096> execLogs;
    int cptOutOfDeadline;
    int cptExecutions;

    void logStart();
    RTIME logExec();
    void saveData(string);
};

struct taskRTInfo
{
  rtTaskInfosStruct* rtTI;
  DataLogger* taskLog;
};


















#endif

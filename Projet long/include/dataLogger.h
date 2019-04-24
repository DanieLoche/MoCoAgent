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
  protected :
    char name[32];
    int id;
    int affinity;
    RTIME deadline;

  public :
    DataLogger();

    std::array<timeLog, 4096> execLogs;
    int cptOutOfDeadline;
    int cptExecutions;

    void logStart(RTIME );
    RTIME logStart();
    void logExec(RTIME );
    RTIME logExec();
    virtual void saveData(string) = 0;
};

class TaskDataLogger : public DataLogger
{
  public :
    RT_TASK* task;
    int isHardRealTime;

    TaskDataLogger(rtTaskInfosStruct*);
    void saveData(string);

};

class ChainDataLogger : public DataLogger
{
  public :
    int cptAnticipatedMisses;

    ChainDataLogger(end2endDeadlineStruct*);
    void saveData(string);

};

struct taskRTInfo
{
  rtTaskInfosStruct* rtTI;
  TaskDataLogger* taskLog;
};

#endif

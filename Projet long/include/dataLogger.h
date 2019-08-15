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
    virtual void saveData(string, int) = 0;

    char* getName();
};

class TaskDataLogger : public DataLogger
{
   private:
      int affinity;
      int priority;
      int isHardRealTime;
   public :
      RT_TASK* task;

      TaskDataLogger(rtTaskInfosStruct*);
      void saveData(string, int);

};

class ChainDataLogger : public DataLogger
{
  public :
    int cptAnticipatedMisses;

    ChainDataLogger(end2endDeadlineStruct*);
    void saveData(string, int);

};

struct taskRTInfo
{
  rtTaskInfosStruct* rtTI;
  TaskDataLogger* taskLog;
};

#endif

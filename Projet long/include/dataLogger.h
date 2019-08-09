#ifndef DATALOGGER_H
#define DATALOGGER_H

#include "tools.h"
#include <array>

struct timeLog
{
  timespec timestamp;
  timespec duration;
};

class DataLogger
{
  protected :
    char name[32];
    int id;
    uint64_t deadline;

  public :
    DataLogger();

    std::array<timeLog, 4096> execLogs;
    int cptOutOfDeadline;
    int cptExecutions;

    void logStart(timespec );
    timespec logStart();
    void logExec(timespec );
    timespec logExec();
    virtual void saveData(string, int) = 0;

    char* getName();
};

class TaskDataLogger : public DataLogger
{
   private:
      int affinity;
   public :
      xnthread* task;
      int isHardRealTime;

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

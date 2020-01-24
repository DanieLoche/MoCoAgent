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
    RTIME deadline;

  public :
    DataLogger();

    std::array<timeLog, 4096> execLogs;
    int cptOutOfDeadline;
    unsigned long overruns;
    int cptExecutions;

    void logStart(RTIME );
    RTIME logStart();
    void logExec(RTIME );
    RTIME logExec();
    virtual void saveData(string, int) = 0;

    virtual char* getName() =0;
};

class TaskDataLogger : public DataLogger
{
   protected:
      rtTaskInfosStruct* taskInfos;
      char* getName();
   public :

      TaskDataLogger(rtTaskInfosStruct*);
      void saveData(string, int);

};

class ChainDataLogger : public DataLogger
{
   protected:
      end2endDeadlineStruct* chainInfos;
      char* getName();
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

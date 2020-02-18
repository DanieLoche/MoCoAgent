#ifndef DATALOGGER_H
#define DATALOGGER_H

#include "tools.h"
#include <array>
#include <iomanip>


struct timeLog
{
   RTIME timestamp;
   RTIME duration;
};

class DataLogger
{
   protected :
      int cptOutOfDeadline;
      RTIME deadline;
      int cptExecutions;
      std::array<timeLog, 4096> execLogs;

      virtual char* getName() =0;

   public :
      unsigned long overruns;
      DataLogger( );

      void logStart(RTIME );
      RTIME logStart();
      void logExec(RTIME );
      RTIME logExec();
      virtual void saveData(string file, int nameSize) = 0;

};

class TaskDataLogger : public DataLogger
{
   protected:
      rtTaskInfosStruct* taskInfos;
      inline char* getName();

   public :
      TaskDataLogger(rtTaskInfosStruct*);
      void saveData(string, int);

};

class ChainDataLogger : public DataLogger
{
   protected:
      end2endDeadlineStruct* chainInfos;
      inline char* getName();

   public :
      int cptAnticipatedMisses;

      ChainDataLogger(end2endDeadlineStruct*);
      void saveData(string, int);

};

/*struct taskRTInfo
{
  rtTaskInfosStruct* rtTI;
  TaskDataLogger* taskLog;
};*/

#endif

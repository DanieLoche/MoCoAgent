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
      string outputFileName;
      int cptOutOfDeadline;
      int cptExecutions;
      std::array<timeLog, 4096> execLogs;
      RTIME deadline;

      virtual char* getName() =0;

   public :
      unsigned long overruns;
      DataLogger(string expeName );

      void logStart(RTIME );
      RTIME logStart();
      void logExec(RTIME );
      RTIME logExec();
      virtual void saveData(int nameSize, RT_TASK_INFO* cti) = 0;
};

class TaskDataLogger : public DataLogger
{
   protected:
      rtTaskInfosStruct taskInfos;
      inline char* getName();

   public :
      TaskDataLogger(rtTaskInfosStruct, string expeName);
      void saveData(int nameSize, RT_TASK_INFO* cti = NULL);

};

class ChainDataLogger : public DataLogger
{
   protected:
      end2endDeadlineStruct chainInfos;
      inline char* getName();

   public :
      int cptAnticipatedMisses;

      ChainDataLogger(end2endDeadlineStruct, string expeName);
      void saveData(int nameSize, RT_TASK_INFO* cti = NULL);

};

/*struct taskRTInfo
{
  rtTaskInfosStruct* rtTI;
  TaskDataLogger* taskLog;
};*/

#endif

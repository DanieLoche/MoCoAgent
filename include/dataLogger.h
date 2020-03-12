#ifndef DATALOGGER_H
#define DATALOGGER_H

#include "tools.h"
#include <array>
#include <iomanip>

#define BUFF_SIZE    4096
struct timeLog
{
   RTIME timestamp;
   RTIME duration;
};

class DataLogger
{
   protected :
      std::array<timeLog, BUFF_SIZE> execLogs;
      int cptOutOfDeadline;
      uint cptExecutions;
      int logPointer;
      RTIME deadline;
      string outputFileName;

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

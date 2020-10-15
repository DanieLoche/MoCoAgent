#ifndef DATALOGGER_H
#define DATALOGGER_H

#include "tools.h"
#include <array>
#include <iomanip>

#define BUFF_SIZE    4096
#define LOG_VALUES_REMOVAL ((cptExecutions <= BUFF_SIZE && cptExecutions >= 30)?10:0)    // we ignore the first xx values from the log buffer for output.
#define MASK_SIZE    0xfff

//=========================//
//=== Task Data Logger ===//
struct timeLog
{
   RTIME timestamp;
   unsigned long duration;
};

class DataLogger
{
   protected :
      uint cptOutOfDeadline;
      uint cptExecutions;
      //int logPointer;
      unsigned long deadline;
      string outputFileName;

      virtual char* getName() =0;

   public :
      unsigned long overruns;
      DataLogger(string expeName );

      virtual void saveData(int nameSize, RT_TASK_INFO* cti) = 0;
};

class TaskDataLogger : public DataLogger
{
   protected:
      std::array<timeLog, BUFF_SIZE> execLogs;
      rtTaskInfosStruct taskInfos;
      inline char* getName();

   public :
      TaskDataLogger(rtTaskInfosStruct, string expeName);

      void logStart(RTIME );
      RTIME logStart();
      void logExec(RTIME );
      RTIME logExec();
      void saveData(int nameSize, RT_TASK_INFO* cti = NULL);

};

//=========================//
//=== Chain Data Logger ===//
struct chainLog
{
   RTIME timestamp;
   unsigned long duration;
   unsigned long* rWCETs;
};

class ChainDataLogger : public DataLogger
{
   protected:
      std::array<chainLog, BUFF_SIZE> execLogs;
      end2endDeadlineStruct chainInfos;
      inline char* getName();
      int chainSize;
      int cptWCET;

   public :
      int cptAnticipatedMisses;

      ChainDataLogger(end2endDeadlineStruct, string expeName);
      void logChain(timeLog _execLog);
      void logWCET(unsigned long time);
      void setLogArray(int size);
      void saveData(int nameSize, RT_TASK_INFO* cti = NULL);

};

/*struct taskRTInfo
{
  rtTaskInfosStruct* rtTI;
  TaskDataLogger* taskLog;
};*/

#endif

#include "tools.h"
//#include <array>

#define   TRUE    1
#define   FALSE   0
#define   MODE_OVERLOADED     1
#define   MODE_NOMINAL        -1

const RTIME t_RT = 000*1.0e6;  // time to trigger the Control Agent
const RTIME Wmax = 400*1.0e6;    // next slice max time


/*
Ce qu'il me faut :
On va assigner un ID à chaque task. Cela permet d'identifier une
même tâche allouée à plusieurs chaines par exemple.
Permet aussi de '<' et '>' entre2 tâches pour optimiser sets.
RT_TASK
startTime     // 0 si on donne un end time
endTime       // 0 si on donne un start time
isExecuted    // 1 en conjonction du end time
chaines concernées par la tâche ??

On MaJ :
si startTime (chaine) == 0, alors startTime = currentTime
currentTime =
*/
class taskMonitoringStruct
{
  private :
    RT_MUTEX mtx_taskStatus;
    bool isExecuted;   // Run-time - computed

  public :
    taskMonitoringStruct(rtTaskInfosStruct* rtTaskInfos);

    RT_TASK* xenoTask;
    int id;
    //RTIME endTime;     // Run-time - received
    RTIME deadline;    // Static
    RTIME rwcet;       // Static

    void setState(bool state);
    bool getState();
    //bool operator <(const taskMonitoringStruct& tms) const {return (id < tms.id);}
};


#include "dataLogger.h"
class taskChain
{
  public :
    taskChain(end2endDeadlineStruct _tcDeadline);
    ChainDataLogger* logger;
    int cptAnticipatedMisses;
    //int cptOutOfDeadline;
    //int cptExecutions;
    //std::array<RTIME, 2048> chainExecutionTime;
    char name[32];
    int chainID;                // static
    RTIME end2endDeadline; // static
    RTIME startTime;       // Runtime - deduced
    RTIME currentEndTime;  // Runtime - deduced
    RTIME remWCET;         // Runtime - computed
    bool isAtRisk;         // Runtime - deduced
    //double slackTime;
    std::vector<taskMonitoringStruct> taskList;

    bool checkTaskE2E();
    bool checkIfEnded();
    void resetChain();
    RTIME getExecutionTime();
    RTIME getRemWCET();

    void displayTasks();
};

class MCAgent
{
  public :
    MCAgent(systemRTInfo* sInfos);
    RT_BUFFER bf;
    RT_EVENT mode_change_flag;
    void updateTaskInfo(monitoringMsg msg);
    void setMode(int mode);
    void saveData(string);

  private :
      bool* triggerSave;
      int runtimeMode;    // NOMINAL or OVERLOADED
      //std::vector<rtTaskInfosStruct>* TasksInformations;
      std::vector<taskChain*> allTaskChain;
      std::vector<RT_TASK*> bestEffortTasks;

      void initMoCoAgent(systemRTInfo* sInfos);
      void initCommunications();
      void setAllDeadlines(std::vector<end2endDeadlineStruct> _tcDeadlineStructs);
      void setAllTasks(std::vector<rtTaskInfosStruct> _TasksInfos);
      //int checkTaskChains();
      void displaySystemInfo(systemRTInfo* sInfos);
      void displayChains();
};

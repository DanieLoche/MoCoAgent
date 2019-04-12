#include "tools.h"
#include <array>

#define   TRUE    1
#define   FALSE   0
#define   MODE_OVERLOADED     1
#define   MODE_NOMINAL        0

const RTIME t_RT = 0*1.0e6;  // time to trigger the Control Agent
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
  public :
    taskMonitoringStruct(rtTaskInfosStruct rtTaskInfos);

    RT_TASK* tache;
    int id;
    RTIME startTime;   // Run-time - received
    RTIME endTime;     // Run-time - received
    bool isExecuted;   // Run-time - computed
    RTIME deadline;    // Static
    RTIME rwcet;       // Static

    //bool operator <(const taskMonitoringStruct& tms) const {return (id < tms.id);}
};

class taskChain
{
  public :
    taskChain(end2endDeadlineStruct _tcDeadline);
    int cptAnticipatedMisses;
    int cptOutOfDeadline;
    int cptExecutions;
    std::array<RTIME, 2048> chainExecutionTime;

    int id;                // static
    bool isAtRisk;         // Runtime - deduced
    RTIME startTime;       // Runtime - deduced
    RTIME currentEndTime;  // Runtime - deduced
    RTIME remWCET;         // Runtime - computed
    RTIME end2endDeadline; // static
    //double slackTime;
    std::vector<taskMonitoringStruct> taskList;

    int checkTaskE2E();
    int checkIfEnded();
    void resetChain();
    RTIME getExecutionTime();
    RTIME getRemWCET();

    //bool operator <(const taskChain& tc) const {return (id < tc.id);}

    void displayTasks();
};

class MCAgent
{
  public :
    MCAgent(systemRTInfo* sInfos);
    RT_BUFFER bf;
    RT_EVENT mode_change_flag;
    void updateTaskInfo(monitoringMsg msg);

  private :
    int runtimeMode;    // NOMINAL or OVERLOADED
    //std::vector<rtTaskInfosStruct>* TasksInformations;
    std::vector<taskChain> allTaskChain;
    std::vector<RT_TASK*> bestEffortTasks;

    void initMoCoAgent(systemRTInfo* sInfos);
    void initCommunications();
    void setAllDeadlines(std::vector<end2endDeadlineStruct> _tcDeadlineStructs);
    void setAllTasks(std::vector<rtTaskInfosStruct> _TasksInfos);
    int checkTaskChains();
    void setMode(int mode);
    void displaySystemInfo(systemRTInfo* sInfos);
    void displayChains();
};

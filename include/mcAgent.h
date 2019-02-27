#ifndef MOCOAGENT_H
#define MOCOAGENT_H

#include "tools.h"
#include <set>
#define   TRUE    1
#define   FALSE   0
#define   MODE_OVERLOADED     1
#define   MODE_NOMINAL        0

const double t_RT = 0;  // time to trigger the Control Agent
const double Wmax = 0;    // next slice max time

struct monitoringMsg
{
  RT_TASK* task;
  int id;
  double startTime;   // Run-time - received
  double endTime;     // Run-time - received
  bool isExecuted;    // Run-time - computed
};
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

    RT_TASK* task;
    int id;
    double startTime;   // Run-time - received
    double endTime;     // Run-time - received
    bool isExecuted;    // Run-time - computed
    double deadline;    // Static
    double rwcet;       // Static

    //bool operator <(const taskMonitoringStruct& tms) const {return (id < tms.id);}
};

class taskChain
{
  public :
    taskChain(end2endDeadlineStruct _tcDeadline);

    int id;                 // static
    bool isAtRisk;          // Runtime - deduced
    double startTime;       // Runtime - deduced
    double currentEndTime;  // Runtime - deduced
    double remWCET;         // Runtime - computed
    double end2endDeadline; // static
    //double slackTime;
    std::vector<taskMonitoringStruct*> taskList;

    int checkTaskE2E();
    int checkIfEnded();
    void resetChain();
    double getExecutionTime();
    double getRemWCET();

    bool operator <(const taskChain& tc) const {return (id < tc.id);}
};

class MCAgent
{
  public :
    MCAgent(void* arg);
    RT_BUFFER bf;
    RT_EVENT mode_change_flag;
    void updateTaskInfo(monitoringMsg msg);

  private :
    int runtimeMode;    // NOMINAL or OVERLOADED
    //std::vector<rtTaskInfosStruct>* TasksInformations;
    std::vector<taskChain> allTaskChain;
    std::vector<RT_TASK*> bestEffortTasks;

    void initMoCoAgent(systemRTInfo* sInfos);
    void initComunications();
    void setAllDeadlines(std::vector<end2endDeadlineStruct> _tcDeadlineStructs);
    void setAllTasks(std::vector<rtTaskInfosStruct> _TasksInfos);
    int checkTaskChains();
    void setMode(int mode);
    void displaySystemInfo(systemRTInfo* sInfos);

};


#endif

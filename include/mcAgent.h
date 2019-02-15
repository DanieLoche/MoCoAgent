#include "tools.h"

static double offset;  // time to trigger the Control Agent
static double Wmax;    // next slice max time

class taskMonitoringStruct
{
  public :
    taskMonitoringStruct(rtTaskInfosStruct rtTaskInfos);

    RT_TASK* task;
    double startTime;
    double endTime;
    double deadline;
    bool isExecuted;
    double rwcet;

};

class taskChain
{
  public :
    taskChain(end2endDeadlineStruct _tcDeadline);

    int id;                // static
    double startTime;
    double currentEndTime;
    double remWCET;
    double end2endDeadline; // static
    //double slackTime;
    std::vector<taskMonitoringStruct> taskList;

    int checkTaskE2E();
    double getExecutionTime();
    // to do : on a le startTime, et on update le endTime après chaque exécution
    // a voir comment... ExecutionTime = end - start.
    double getRemWCET();
    // to do : On parcours toutes les tâches, on somme les deadlines
    // de toutes les tâches non exécutées.
};

class MCAgent
{
  public :
    MCAgent(void* arg);

  private :
    int runtimeMode;
    int triggerCount;
    //std::vector<rtTaskInfosStruct>* TasksInformations;
    std::vector<taskChain> allTaskChain;
    std::vector<RT_TASK*> bestEffortTasks;

    void initMoCoAgent(systemRTInfo* sInfos);
    void setAllDeadlines(std::vector<end2endDeadlineStruct> _tcDeadlineStructs);
    void setAllTasks(std::vector<rtTaskInfosStruct> _TasksInfos);
    int checkTaskChains();
    void setMode(int mode);
    void displaySystemInfo(systemRTInfo* sInfos);

};

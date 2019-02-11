#include "tools.h"

class taskMonitoringStruct
{
  public :
    RT_TASK* task;
    double execTime;
    double deadline;
    bool isExecuted;
    double rwcet;

    taskMonitoringStruct(rtTaskInfosStruct rtTaskInfos);
    taskMonitoringStruct(RT_TASK* t, double d, double r)
                          { task = t; deadline = d; rwcet = r;}

};

class taskChain
{
  public :
    taskChain(end2endDeadlineStruct _tcDeadline);
    taskChain(int _id, double _deadline);

    int id;
    double end2endDeadline;
    double slackTime;
    std::vector<taskMonitoringStruct> taskChainList;

    void setTaskChain(std::vector<rtTaskInfosStruct> rtTasks);
    int checkTaskE2E();
};

class MCAgent
{
  private :
    int runtimeMode;
    //std::vector<rtTaskInfosStruct>* TasksInformations;
    std::vector<taskChain> allTaskChain;
  public :
    MCAgent(void* arg);

    void initMoCoAgent(std::vector<rtTaskInfosStruct>* sInfos);
    void displayInformations();
    int checkTasks();
    void setAllDeadlines(std::vector<end2endDeadlineStruct> _tcDeadlineStructs);
    void setAllTasks(std::vector<rtTaskInfosStruct>* _TasksInfos);
    void setMode(int mode);
};

#include "tools.h"

class taskMonitoringStruct
{
  public :
    RT_TASK* task;
    double execTime;
    double deadline;
    bool isExecuted;
    double rwcet;

  taskMonitoringStruct(RT_TASK* t, double d, double r)
    { task = t; deadline = d; rwcet = r;}

};

class taskChain
{
  public :
    std::vector<taskMonitoringStruct> taskChainList;
    double end2endDeadline;

    void setTaskChain(std::vector<rtTaskInfosStruct> rtTasks)
      { for (auto& task : rtTasks)
        {
            taskMonitoringStruct tMStruct(task.task, task.deadline, task.periodicity);
            taskChainList.push_back(tMStruct);
        }
      }

    int checkTaskRT()
      {

      }
};

class MCAgent
{
  private :
    int runtimeMode;
    std::vector<rtTaskInfosStruct>* TasksInformations;
    std::vector<taskChain> allTaskChain;
  public :
    MCAgent(void* arg);
    void displayInformations();
    int checkTasks();
    void setMode(int mode);
};

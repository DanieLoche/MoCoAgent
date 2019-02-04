
#include "tools.h"

class MCAgent
{
  private :
    int taskListSize;
    RT_TASK *taskList;
    std::vector<rtTaskInfosStruct>* TasksInformations;

  public :
    MCAgent(void* arg);
    void setTasksInfos(std::vector<rtTaskInfosStruct>* _tasksInfos);
    void displayInformations();
    int checkTasks();
};

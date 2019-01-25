#include "tools.h"

class MCAgent
{
  private :
    int taskListSize;
    RT_TASK *taskList;
    vector<rtTaskInfosStruct> TasksInformations;
  public :
    MCAgent();
    void mcAgentMain(void *arg);

};

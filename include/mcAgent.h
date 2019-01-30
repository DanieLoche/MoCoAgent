
#include "tools.h"

class MCAgent
{
  private :
    int taskListSize;
    RT_TASK *taskList;
    std::vector<rtTaskInfosStruct> TasksInformations;
  public :
    MCAgent();
    void mcAgentMain(void *arg);

};

#include "tools.h"

class MCAgent
{
  private :
    int taskListSize;
    RT_TASK *taskList;
  public :
    MCAgent();
    void mcAgentMain(void *arg);

};

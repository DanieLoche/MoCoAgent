#include "tools.h"

extern void RunmcAgentMain(void *arg);
extern void TaskMain(void* arg);

class TaskLauncher
{
  public :
    std::vector<rtTaskInfosStruct> tasksInfosList;


    TaskLauncher(string input_file);

    void runTasks( );
    std::vector<rtTaskInfosStruct> readTasksList(string);

    int set_affinity (RT_TASK* task, int _aff);

    void printTasksInfos ( );
    void printTaskInfo(rtTaskInfosStruct*);
    void print_affinity(pid_t _pid);

};

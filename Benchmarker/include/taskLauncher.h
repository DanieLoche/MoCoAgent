#include "tools.h"

class TaskLauncher
{
  public :
    std::vector<rtTaskInfosStruct> tasksInfosList;


    TaskLauncher(string input_file,int);

    void runTasks();
    std::vector<rtTaskInfosStruct> readTasksList(string,int);

    void set_affinity (RT_TASK* task, int _aff);

    void printTasksInfos (/* std::vector<rtTaskInfosStruct> _myTasksInfos*/);
    void print_affinity(pid_t _pid);

};

extern void RunmcAgentMain(void *arg);
extern void TaskMain(void* arg);
extern void print_affinity(pid_t _pid);
extern void printTaskInfo(rtTaskInfosStruct* task);

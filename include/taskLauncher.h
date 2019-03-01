#include "tools.h"



class TaskLauncher
{

  private :
  int number_task_created ;
  public :
    std::vector<rtTaskInfosStruct> tasksInfosList;

    TaskLauncher();
    TaskLauncher(string input_file);

    void runTasks( );
    std::vector<rtTaskInfosStruct> readTasksList(string);

    void printTasksInfos (/* std::vector<rtTaskInfosStruct> _myTasksInfos*/);
    void print_affinity(pid_t _pid);

};

extern void RunmcAgentMain(void *arg);
extern void TaskMain(void* arg);
extern void print_affinity(pid_t _pid);
extern void printTaskInfo(rtTaskInfosStruct* task);
extern void set_affinity (RT_TASK* task, int _aff);

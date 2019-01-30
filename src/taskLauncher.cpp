#include <sys/sysinfo.h>

#include "taskLauncher.h"

TaskLauncher::TaskLauncher(string input_file)
{
  tasksInfosList = readTasksList(input_file);

      RT_TASK task;
      char   name[64] = "MoCoAgent";
      rt_task_create(&task, name, 0, 30, 0);
      cout << "Monitoring and Control Agent created." << endl;
      set_affinity(&task, 2);

      cout << "Task " << name << " started." << endl;
      int rep = rt_task_start(&task, &RunmcAgentMain, NULL);

}


std::vector<rtTaskInfosStruct> TaskLauncher::readTasksList(string input_file)
{
  system("clear");
  cout << "Initialising machine...\n";
  std::ifstream myFile(input_file);
  if (!myFile.is_open())
  {
      exit(EXIT_FAILURE);
  }

  //std::vector<rtTaskInfosStruct> myTasksInfos;

  string str;
  std::getline(myFile, str); // skip the first line
  while (std::getline(myFile, str))
  {
      rtTaskInfosStruct taskInfo;
      std::istringstream iss(str);
      string token;
      cout << "Managing line : " << str << endl;
      if (!(iss >> taskInfo.name
                >> taskInfo.path
                >> taskInfo.isHardRealTime
                >> taskInfo.periodicity
                >> taskInfo.deadline
                >> taskInfo.affinity) )
      { cout << "FAIL !" << endl; break; } // error
      tasksInfosList.push_back(taskInfo);
  }

  return tasksInfosList;

}

void TaskLauncher::printTasksInfos (/* std::vector<rtTaskInfosStruct> _myTasksInfos*/)
{
  for (auto &taskInfo : tasksInfosList)
  {
      cout << "Name: " << taskInfo.name
          << "| path: " << taskInfo.path
          << "| is RT ? " << taskInfo.isHardRealTime
          << "| Period: " << taskInfo.periodicity
          << "| Deadline: " << taskInfo.deadline
          << "| affinity: " << taskInfo.affinity << endl;
  }
}

void TaskLauncher::printTaskInfo(rtTaskInfosStruct* task)
{
  cout << "Name: " << task->name
      << "| path: " << task->path
      << "| is RT ? " << task->isHardRealTime
      << "| Period: " << task->periodicity
      << "| Deadline: " << task->deadline
      << "| affinity: " << task->affinity << endl;
}

void TaskLauncher::runTasks( )
{

  for (auto taskInfo = tasksInfosList.begin(); taskInfo != tasksInfosList.end(); ++taskInfo)
  {
      RT_TASK* task = new RT_TASK;
      taskInfo->task = task;
      rt_task_create(task, taskInfo->name, 0, 50, 0);
      cout << "Task " << taskInfo->name << " created." << endl;
      set_affinity(task, 1);
      /*
      RT_TASK_INFO rti;
      rt_task_inquire(task, &rti);
      print_affinity(rti.pid);
      */
  }

  for (auto taskInfo = tasksInfosList.begin(); taskInfo != tasksInfosList.end(); ++taskInfo)
  {
      cout << "Task " << taskInfo->name << " started." << endl;
      int rep = rt_task_start(taskInfo->task, TaskMain, &taskInfo);
  }
}

int TaskLauncher::set_affinity (RT_TASK* task, int _aff)
{
  cpu_set_t mask;
  CPU_ZERO(&mask);
  CPU_SET(_aff, &mask);
  return rt_task_set_affinity(task, &mask);
}

void TaskLauncher::print_affinity(pid_t _pid) {
    cpu_set_t mask;
    long nproc, i;

    if (sched_getaffinity(_pid, sizeof(cpu_set_t), &mask) == -1) {
        perror("sched_getaffinity");
        assert(false);
    } else {
        nproc = get_nprocs();
        cout << "Affinity of thread " << _pid << " = ";
        for (i = 0; i < nproc; i++) {
            cout << CPU_ISSET(i, &mask);
        }
        cout << endl;
        /* using printf
        printf("sched_getaffinity = ");
        for (i = 0; i < nproc; i++) {
            printf("%d ", CPU_ISSET(i, &mask));
        }
        printf("\n");
        */
    }

}

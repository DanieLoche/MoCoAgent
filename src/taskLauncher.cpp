#include "taskLauncher.h"
#include "sched.h"

TaskLauncher::TaskLauncher(string input_file)
{
  tasksInfosList = readTasksList(input_file);
}


std::vector<rtTaskInfosStruct> TaskLauncher::readTasksList(string input_file)
{
  #if VERBOSE_OTHER
  system("clear");
  rt_printf("Initialising machine...\n");
  //cout << "Initialising machine...\n";
  #endif
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
      #if VERBOSE_ASK
      cout << "Managing line : " << str << endl;
      #endif
      if (str.substr(0,2) != "//")
      {
        if (!(iss >> taskInfo.name
                  >> taskInfo.path
                  >> taskInfo.isHardRealTime
                  >> taskInfo.periodicity
                  >> taskInfo.deadline
                  >> taskInfo.affinity) )
        { cout << "FAIL !" << endl; break; } // error
        tasksInfosList.push_back(taskInfo);
      }
      #if VERBOSE_ASK
      else
        cout << "line ignored." << endl;
      #endif
  }

  return tasksInfosList;

}


void TaskLauncher::runTasks( )
{


  for (auto taskInfo = tasksInfosList.begin(); taskInfo != tasksInfosList.end(); ++taskInfo)
  {
      RT_TASK* task = new RT_TASK;
      taskInfo->task = task;
      rt_task_create(task, taskInfo->name, 0, 50, 0);
      set_affinity(task, 0);
      #if VERBOSE_INFO
      cout << "Task " << taskInfo->name << " created." << endl;
      #endif

      /*
      RT_TASK_INFO rti;
      rt_task_inquire(task, &rti);
      print_affinity(rti.pid);
      */
  }

  for (auto& taskInfo : tasksInfosList)
  {
    #if VERBOSE_INFO
      cout << "Task " << taskInfo.name << " started." << endl;
    #endif
      int rep = rt_task_start(taskInfo.task, TaskMain, &taskInfo);
  }

  #if VERBOSE_OTHER
    cout << "Now launching the MoCoAgent ! " << endl;
  #endif
  RT_TASK mcAgent;
  int rep = rt_task_create(&mcAgent, "MoCoAgent", 0, 2, 0);
  set_affinity(&mcAgent, 3);

  rt_task_start(&mcAgent, RunmcAgentMain, &tasksInfosList);

}

void TaskLauncher::set_affinity (RT_TASK* task, int _aff)
{
  cpu_set_t mask;
  CPU_ZERO(&mask);
  CPU_SET(_aff, &mask);
  rt_task_set_affinity(task, &mask);

}

void TaskLauncher::printTasksInfos (/* std::vector<rtTaskInfosStruct> _myTasksInfos*/)
{
  #if VERBOSE_INFO
  for (auto &taskInfo : tasksInfosList)
  {
      cout << "Name: " << taskInfo.name
          << "| path: " << taskInfo.path
          << "| is RT ? " << taskInfo.isHardRealTime
          << "| Period: " << taskInfo.periodicity
          << "| Deadline: " << taskInfo.deadline
          << "| affinity: " << taskInfo.affinity << endl;
  }
  #endif
}

#include "taskLauncher.h"
#include "sched.h"

TaskLauncher::TaskLauncher(string input_file)
{
  int ret = readTasksList(input_file);
  if (ret != 0)
  {
    cout << "Failed to read whole file." << endl;
    exit(EXIT_FAILURE);
    delete this;
  }
}


int TaskLauncher::readTasksList(string input_file)
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

      std::istringstream iss(str);
      #if VERBOSE_ASK
      cout << "Managing line : " << str << endl;
      #endif
      if (str.substr(0,2) == "$$")
      {
        end2endDeadlineStruct e2e;
        if (!(iss >> e2e.taskChainID >> e2e.taskChainID
                  >> e2e.deadline) )
        { cout << "FAIL !" << endl; return 10; } // error
        rtInfos.e2eDD->push_back(e2e);
      }
      else if (str.substr(0,2) != "//")
      {
        rtTaskInfosStruct taskInfo;
        if (!(iss >> taskInfo.name
                  >> taskInfo.path
                  >> taskInfo.isHardRealTime
                  >> taskInfo.periodicity
                  >> taskInfo.deadline
                  >> taskInfo.affinity) )
        { cout << "FAIL !" << endl; return 20; } // error
        rtInfos.rtTIs->push_back(taskInfo);
      }
      #if VERBOSE_ASK
      else
        cout << "line ignored." << endl;
      #endif
  }

  return 0;

}


void TaskLauncher::runTasks( )
{

  for (auto taskInfo = tasksInfosList.begin(); taskInfo != tasksInfosList.end(); ++taskInfo)
  {
      RT_TASK* task = new RT_TASK;
      taskInfo->task = task;
      rt_task_create(task, taskInfo->name, 0, 50, 0);
      //set_affinity(task, 0);
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

#include "taskLauncher.h"
#include "sched.h"
#include "tools.h"

TaskLauncher::TaskLauncher(string input_file,int iteration)
{
  tasksInfosList = readTasksList(input_file,iteration);
}


std::vector<rtTaskInfosStruct> TaskLauncher::readTasksList(string input_file,int iteration)
{
  system("clear");
  cout << "Initialising machine..." << endl; cout.flush();
  std::ifstream myFile(input_file);
  if (!myFile.is_open())
  {
      exit(EXIT_FAILURE);
  }

  //std::vector<rtTaskInfosStruct> myTasksInfos;

  string str;
  int i;
  for (i=0 ; i<iteration ; i++){
    std::getline(myFile, str); // skip lines
  }
  rtTaskInfosStruct taskInfo;
  std::istringstream iss(str);
  string token;
  cout << "Managing line : " << str << endl; cout.flush();
  if (str.substr(0,2) != "//")
  {
    if (!(iss >> taskInfo.name
              >> taskInfo.path
              >> taskInfo.isHardRealTime
              >> taskInfo.periodicity
              >> taskInfo.deadline
              >> taskInfo.affinity))
    { cout << "FAIL !" << endl; cout.flush();} // error
    tasksInfosList.push_back(taskInfo);
    } else cout << "line ignored." << endl; cout.flush();

  return tasksInfosList;
}


void TaskLauncher::runTasks( )
{

for (auto& taskInfo : tasksInfosList)  {
      RT_TASK* task = new RT_TASK;
      taskInfo.task = task;
      rt_task_create(task, taskInfo.name, 0, 50, 0);
      cout << "Task " << taskInfo.name << " created." << endl; cout.flush();
      set_affinity(task, 0);
      /*
      RT_TASK_INFO rti;
      rt_task_inquire(task, &rti);
      print_affinity(rti.pid);
      */
  }

  for (auto& taskInfo : tasksInfosList)
  {
      cout << "Task " << taskInfo.name << " started." << endl; cout.flush();
      cout << "Task " << taskInfo.task << " ready." << endl;
      int rep = rt_task_start(taskInfo.task, TaskMain, &taskInfo);
  }

  sleep(2);

  for (auto& taskInfo : tasksInfosList)
  {
    cout << "Task " << taskInfo.name << " stopped." << endl; cout.flush();
    cout << "Task " << taskInfo.task << " stopped." << endl;
    int delrep = rt_task_delete(taskInfo.task);
  }

}

void TaskLauncher::set_affinity (RT_TASK* task, int _aff)
{
  cpu_set_t mask;
  CPU_ZERO(&mask);
  CPU_SET(_aff, &mask);
  /*RT_TASK_INFO tsk_infos;
  rt_task_inquire(0, &tsk_infos);*/
  cout << "Setting affinity :" << rt_task_set_affinity(task, &mask) << endl; cout.flush();
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
          << "| affinity: " << taskInfo.affinity << endl; cout.flush();
  }
}

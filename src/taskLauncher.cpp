#include "taskLauncher.h"
#include "sched.h"

#define NTASKS 4

RT_TASK demo_task[NTASKS];

TaskLauncher::TaskLauncher(string input_file)
{
  tasksInfosList = readTasksList(input_file);
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
      } else cout << "line ignored." << endl;
  }

  return tasksInfosList;

}

void TaskLauncher::runTasks( )
{
   SRTIME quant=10e7;
   SRTIME qt = rt_timer_ns2ticks(quant);

  for (auto taskInfo = tasksInfosList.begin(); taskInfo != tasksInfosList.end(); ++taskInfo)
  {
      RT_TASK* task = new RT_TASK;
      taskInfo->task = task;
      rt_task_create(task, taskInfo->name, 0, 50, 0);
      cout << "Task " << taskInfo->name << " created." << endl;
      set_affinity(task, 0);
    //  rt_task_slice(task,qt);
     /*
      RT_TASK_INFO rti;
      rt_task_inquire(task, &rti);
      print_affinity(rti.pid);
      */
  }

   //Periodicity
  RTIME starttime,period;
  starttime = TM_NOW ;

  for (auto& taskInfo : tasksInfosList)
  {
      period = taskInfo.periodicity*1e8 ;
      rt_task_set_periodic(taskInfo.task, starttime, period);

      //Starting
      cout << "Task " << taskInfo.name << " started.at =" << starttime <<endl;
      int rep = rt_task_start(taskInfo.task, TaskMain, &taskInfo);
  }


}

void TaskLauncher::set_affinity (RT_TASK* task, int _aff)
{
  cpu_set_t mask;
  CPU_ZERO(&mask);
  CPU_SET(_aff, &mask);
  /*RT_TASK_INFO tsk_infos;
  rt_task_inquire(0, &tsk_infos);*/
  cout << "Setting affinity :" << rt_task_set_affinity(task, &mask) << endl;
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

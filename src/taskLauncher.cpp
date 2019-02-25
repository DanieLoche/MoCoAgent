#include "taskLauncher.h"
#include "sched.h"


TaskLauncher::TaskLauncher(string input_file, int chaineID)
{
  tasksInfosList = readTasksList(input_file, chaineID);
}


std::vector<rtTaskInfosStruct> TaskLauncher::readTasksList(string input_file, int chaineID)
{
  //system("clear");
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
                  >> taskInfo.affinity
                  ) )
        { cout << "\033[1;31mFailed to read line\033[0m !" << endl; break; } // error
        tasksInfosList.push_back(taskInfo);
      }
      #if VERBOSE_ASK
       else cout << "line ignored." << endl;
      #endif
  }

  return tasksInfosList;

}


void TaskLauncher::runTasks( )
{
   SRTIME quant=1e7;
   SRTIME qt = rt_timer_ns2ticks(quant);
  /* cpu_set_t mask;
   CPU_ZERO(&mask);
   CPU_SET(0, &mask);
   CPU_SET(1, &mask);
   CPU_SET(2, &mask);
   CPU_SET(3, &mask);
   CPU_SET(4, &mask);
   CPU_SET(5, &mask);
   CPU_SET(6, &mask);
   CPU_SET(7, &mask);
*/
    //  rt_task_set_mode(0,XNRRB,NULL);

  for (auto taskInfo = tasksInfosList.begin(); taskInfo != tasksInfosList.end(); ++taskInfo)
  {
      RT_TASK* task = new RT_TASK;
      taskInfo->task = task;
      rt_task_create(task, taskInfo->name, 0, 50, 0);
      set_affinity(task, taskInfo->affinity);

      #if VERBOSE_INFO
      cout << "Task " << taskInfo->name << " created." << endl;
      #endif
      set_affinity(task, taskInfo->affinity);

      //cout << "Setting affinity :" << rt_task_set_affinity(taskInfo->task, &mask) << endl;
    //  rt_task_slice(task,qt);

  }

   //Periodicity
  RTIME starttime;
  starttime = TM_NOW ;
  RT_TASK_INFO curtaskinfo;

  for (auto& taskInfo : tasksInfosList)
  {
      taskInfo.deadline = taskInfo.deadline*1e6;
      rt_task_set_periodic(NULL, starttime, taskInfo.periodicity*1e6);
      rt_task_inquire(taskInfo.task, &curtaskinfo);
/*
     cout << "getting affinity :" << sched_getaffinity(curtaskinfo.pid,sizeof(cpu_set_t),&mask) << endl;
     cout<<"nyum cpu   : "<< CPU_COUNT(&mask) <<endl;
     cout<<" cpu   : "<< CPU_ISSET(0,&mask) << CPU_ISSET(1,&mask) <<CPU_ISSET(2,&mask) <<CPU_ISSET(3,&mask) <<CPU_ISSET(4,&mask) <<CPU_ISSET(5,&mask) << CPU_ISSET(6,&mask) << CPU_ISSET(7,&mask) <<endl;
      */
      struct sched_attr para;

      para.sched_policy = SCHED_RR;
      para.sched_flags= SCHED_FLAG_RESET_ON_FORK	;
      //para.sched_runtime= taskInfo.deadline;;
      //para.sched_deadline=taskInfo.deadline;
      //para.sched_period = period;
      para.sched_priority = 50 ;
      para.size=sizeof(sched_attr);

      if( sched_setattr(curtaskinfo.pid,&para,0) != 0) {
        fprintf(stderr,"error setting scheduler ... are you root? : %d \n", errno);
        exit(0);
      }
      #if VERBOSE_INFO
      //Starting
      cout << "Task " << taskInfo.name << " started at = " << starttime <<endl;
      #endif

      /*int rep =*/ rt_task_start(taskInfo.task, TaskMain, &taskInfo);

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
  RT_TASK_INFO curtaskinfo;
  rt_task_inquire(task, &curtaskinfo);
  cout << "Setting affinity for task " << curtaskinfo.name << " : CPU" << rt_task_set_affinity(task, &mask) << endl;
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

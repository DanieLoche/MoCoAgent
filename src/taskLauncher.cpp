#include "taskLauncher.h"
#include "sched.h"



#define SCHED_DEADLINE  6
#define SCHED_FLAG_RESET_ON_FORK	0x01

/* __NR_sched_setattr number */
#ifndef __NR_sched_setattr
#ifdef __x86_64__
#define __NR_sched_setattr      314
#endif

#ifdef __i386__
#define __NR_sched_setattr      351
#endif

#ifdef __arm__
#define __NR_sched_setattr      380
#endif

#ifdef __aarch64__
#define __NR_sched_setattr      274
#endif
#endif

/* __NR_sched_getattr number */
#ifndef __NR_sched_getattr
#ifdef __x86_64__
#define __NR_sched_getattr      315
#endif

#ifdef __i386__
#define __NR_sched_getattr      352
#endif

#ifdef __arm__
#define __NR_sched_getattr      381
#endif

#ifdef __aarch64__
#define __NR_sched_getattr      275
#endif
#endif

struct sched_attr {
    __u32 size;

    __u32 sched_policy;
    __u64 sched_flags;

    /* SCHED_NORMAL, SCHED_BATCH */
    __s32 sched_nice;

    /* SCHED_FIFO, SCHED_RR */
    __u32 sched_priority;

    /* SCHED_DEADLINE */
    __u64 sched_runtime;
    __u64 sched_deadline;
    __u64 sched_period;
};

int sched_setattr(pid_t pid,
              const struct sched_attr *attr,
              unsigned int flags)
{
    return syscall(__NR_sched_setattr, pid, attr, flags);
}

int sched_getattr(pid_t pid,
              struct sched_attr *attr,
              unsigned int size,
              unsigned int flags)
{
    return syscall(__NR_sched_getattr, pid, attr, size, flags);
}








TaskLauncher::TaskLauncher()
{
number_task_created =0;
}


TaskLauncher::TaskLauncher(string input_file)
{
  tasksInfosList = readTasksList(input_file);
  number_task_created =0;
}


std::vector<rtTaskInfosStruct> TaskLauncher::readTasksList(string input_file)
{
  //system("clear");
  cout << "Initialising machine...\n";
  cout << "reading file :"<< input_file << endl;

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
                  >> taskInfo.path_task
                  >> taskInfo.isHardRealTime
                  >> taskInfo.periodicity
                  >> taskInfo.deadline
                  >> taskInfo.affinity  ) )
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
  for (auto taskInfo = tasksInfosList.begin(); taskInfo != tasksInfosList.end(); ++taskInfo)
  {
      RT_TASK* task = new RT_TASK;
      taskInfo->task = task;
      taskInfo->ID = number_task_created ;
      number_task_created = number_task_created + 1 ;
      taskInfo->deadline = taskInfo->deadline*1e6;
      int rep =rt_task_create(task, taskInfo->name, 0, 50, 0);
      if( 0 > rep)
      {
        printf("fail creat task %s\n",taskInfo->name);
      }


      #if VERBOSE_INFO
      cout << "Task " << taskInfo->name << " created." << endl;
      #endif
      set_affinity(taskInfo->task , taskInfo->affinity);


  }

   //Periodicity
  RTIME starttime;
  starttime = TM_NOW ;

  for (auto& taskInfo : tasksInfosList)
  {

      rt_task_set_periodic(taskInfo.task, starttime, taskInfo.periodicity*1e6);
/*
      rt_task_inquire(taskInfo.task, &curtaskinfo);
      struct sched_attr para;
      para.sched_policy = SCHED_FIFO;
      para.sched_flags= SCHED_FLAG_RESET_ON_FORK	;
      //para.sched_runtime= taskInfo.deadline;;
      //para.sched_deadline=taskInfo.deadline;
      //para.sched_period = period;
      para.sched_priority = 50 ;
      para.size=sizeof(sched_attr);

      if( sched_setattr(curtaskinfo.pid,&para,0) != 0) {
        fprintf(stderr,"error setting scheduler ... are you root? : %d \n", errno);
        exit(0);
      }*/
      #if VERBOSE_INFO
      //Starting
      cout << "Task " << taskInfo.name << " started at = " << starttime <<endl;
      #endif

      int rep = rt_task_start(taskInfo.task, TaskMain, &taskInfo);
      if( 0 > rep)
      {
        printf("fail start task %s\n",taskInfo.name);
      }

  }

}


void TaskLauncher::printTasksInfos (/* std::vector<rtTaskInfosStruct> _myTasksInfos*/)
{
  for (auto &taskInfo : tasksInfosList)
  {
      cout << "Name: " << taskInfo.name
          << "| path: " << taskInfo.path_task
          << "| is RT ? " << taskInfo.isHardRealTime
          << "| Period: " << taskInfo.periodicity
          << "| Deadline: " << taskInfo.deadline
          << "| affinity: " << taskInfo.affinity
          << "| ID :"<< taskInfo.ID << endl;

  }
}

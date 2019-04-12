#include "taskLauncher.h"
#include "sched.h"
#include "edf.h"
#include "dataLogger.h"


TaskLauncher::TaskLauncher()
{
  cptNumberTasks =0;
}

void TaskLauncher::readChainsList(string input_file)
{
  cout << "Initialising machine...\n";

  std::ifstream myFile(input_file);
  if (!myFile.is_open())
  {
      exit(EXIT_FAILURE);
  }

  string str;
  std::getline(myFile, str); // skip the first line
  while (std::getline(myFile, str))
  {
      end2endDeadlineStruct chaineInfo;
      std::istringstream iss(str);
      string token;
      #if VERBOSE_INFO
      cout << "Managing line : " << str << endl;
      #endif
      if (str.substr(0,2) != "//")
      {
            if (!(iss >> chaineInfo.name
                      >> chaineInfo.taskChainID
                      >> chaineInfo.Num_tasks
                      >> chaineInfo.Path
                      >> chaineInfo.deadline ))
            { cout << "\033[1;31mFailed to read line\033[0m !" << endl; break; } // error
            chaineInfo.deadline *= 1.0e6;
            taskSetInfos.e2eDD.push_back(chaineInfo);
      } else cout << "line ignored." << endl;
  }

}

int TaskLauncher::readTasksList()
{

  for(int i=0; i < (int)taskSetInfos.e2eDD.size(); ++i )
  {
    std::ifstream myFile(taskSetInfos.e2eDD[i].Path);
    if (!myFile.is_open())
    {
        cout << "Failed to open file path : " << taskSetInfos.e2eDD[i].Path
             << endl;
        exit(EXIT_FAILURE);
    }
    //std::vector<rtTaskInfosStruct> myTasksInfos;

    string str;
    std::getline(myFile, str); // skip the first line
    while (std::getline(myFile, str))
    {
        rtTaskInfosStruct* taskInfo = new rtTaskInfosStruct;
        std::istringstream iss(str);
        string token;
        char ext[64] = "RT_";
        #if VERBOSE_INFO
        cout << "Managing line : " << str << endl;
        #endif
        if (str.substr(0,2) != "//")
        {
          char name[60];
          if (!(iss >> name
                    >> taskInfo->path_task
                    >> taskInfo->isHardRealTime
                    >> taskInfo->wcet
                    >> taskInfo->deadline
                    >> taskInfo->affinity
                    >> taskInfo->priority ) )
          { cout << "\033[1;31mFailed to read line\033[0m !" << endl; break; } // error
          taskInfo->deadline *= 1.0e6;
          taskInfo->wcet *= 1.0e6;
          strncat(ext, name, 64);
          strcpy(taskInfo->name,ext);
          taskInfo->id = ++cptNumberTasks;
          //printTaskInfo(&taskInfo);
          taskSetInfos.rtTIs.push_back(*taskInfo);
        }
        #if VERBOSE_ASK
         else cout << "line ignored." << endl;
        #endif
    }
  }
  return 0;

}


void TaskLauncher::runTasks()
{
  #if VERBOSE_INFO
  cout << endl << "CREATING TASKS : " << endl;
  #endif
    for (auto taskInfo = taskSetInfos.rtTIs.begin(); taskInfo != taskSetInfos.rtTIs.end(); ++taskInfo)
    {
        RT_TASK* task = new RT_TASK;
        taskInfo->task = task;
        #if VERBOSE_INFO
        cout << "Creating Task " << taskInfo->name << "." << endl;
        #endif

        if(rt_task_create(task, taskInfo->name, 0, taskInfo->priority, 0) < 0)
        {
          printf("Failed to create task %s\n",taskInfo->name);
        }
        else
        {
          set_affinity(task, taskInfo->affinity);
        }
      //cout << "Setting affinity :" << rt_task_set_affinity(taskInfo->task, &mask) << endl;
      //  rt_task_slice(task,qt);

    }

  //tasksLogsList = new DataLogger*[cptNumberTasks];

  #if VERBOSE_INFO
  cout << endl << "STARTING TASKS : " << endl;
  #endif

     //Periodicity
    RTIME starttime = TM_NOW ;
    RT_TASK_INFO curtaskinfo;

    for (auto& taskInfo : taskSetInfos.rtTIs)
    {
        rt_task_set_periodic(taskInfo.task, starttime, taskInfo.deadline);
        rt_task_inquire(taskInfo.task, &curtaskinfo);

        struct sched_attr para;
        para.sched_policy = SCHED_POLICY;
        para.sched_flags= 0;
        //para.sched_runtime = taskInfo.deadline;;
        //para.sched_deadline = taskInfo.deadline;
        para.sched_period = taskInfo.deadline;
        para.sched_priority = taskInfo.priority;
        para.size=sizeof(sched_attr);

        if( sched_setattr(curtaskinfo.pid, &para, 0) != 0)
        {
          fprintf(stderr,"error setting scheduler ... are you root? : %d \n", errno);
          exit(0);
        }

        DataLogger* dlog = new DataLogger(&taskInfo);
        taskRTInfo* _taskRTInfo = new taskRTInfo;
        _taskRTInfo->taskLog = dlog;
        _taskRTInfo->rtTI = &taskInfo;
        if( rt_task_start(taskInfo.task, TaskMain, _taskRTInfo) < 0)
        {
          printf("Failed to start task %s\n",taskInfo.name);
        }
        else
        {
          #if VERBOSE_INFO
          cout << "Task " << taskInfo.name << " running." << endl;
          #endif
          tasksLogsList.push_back(dlog);
        }
    }
}

void TaskLauncher::runAgent()
{
  #if VERBOSE_INFO
    cout << endl << "LAUNCHING MoCoAgent." << endl;
  #endif

  RT_TASK mcAgent;
  rt_task_create(&mcAgent, "MoCoAgent", 0, 2, 0);
  set_affinity(&mcAgent, 3);

//  systemRTInfo ch_taks ;
  rt_task_start(&mcAgent, RunmcAgentMain, &taskSetInfos);

}

void TaskLauncher::saveData()
{
  string out_file = "ExpeOutput.csv2";
  std::ofstream myFile;
  myFile.open (out_file);    // TO APPEND :  //,ios_base::app);
  myFile << "timestamp ; name ; ID ; HRT ; deadline ; duration ; affinity \n";
  myFile.close();
  for (auto& taskLog : tasksLogsList)
  {
      taskLog->saveData(out_file);
  }

}

void TaskLauncher::printTasksInfos ( ) // std::vector<rtTaskInfosStruct> _myTasksInfos)
{
  #if VERBOSE_INFO
  cout << "Resume of tasks set information : " << endl;
  for (auto &taskInfo : taskSetInfos.rtTIs)
  {
      cout << "Name: " << taskInfo.name
          << "| path: " << taskInfo.path_task
          << "| is RT ? " << taskInfo.isHardRealTime
          << "| Period: " << taskInfo.wcet/1.0e6
          << "| Deadline: " << taskInfo.deadline/1.0e6
          << "| affinity: " << taskInfo.affinity
          << "| ID :"<< taskInfo.id << endl;

  }
  #endif
}

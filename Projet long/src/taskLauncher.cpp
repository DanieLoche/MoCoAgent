#include "taskLauncher.h"
#include "sched.h"
#include "edf.h"
#include "dataLogger.h"


TaskLauncher::TaskLauncher()
{
   cptNumberTasks =0;
   schedPolicy = SCHED_FIFO;
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
         cout << "Failed to open file : " << taskSetInfos.e2eDD[i].Path << endl;
         exit(EXIT_FAILURE);
      }

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
            getline(iss , taskInfo->arguments);
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
      if (schedPolicy == SCHED_RM)
      { // Changer les niveaux de priorité si on schedule en RM.
         cout << "Updating task informations to use Rate-Monotinic Scheduling." << endl;
         std::sort(taskSetInfos.rtTIs.begin(), taskSetInfos.rtTIs.end(), sortAscendingDeadline());
         int prio = 1;
         RTIME lastPeriod = taskSetInfos.rtTIs[0].deadline;
         for (auto taskInfo = taskSetInfos.rtTIs.begin(); taskInfo != taskSetInfos.rtTIs.end(); ++taskInfo)
         {
            if (taskInfo->deadline != lastPeriod)
            {
               prio++;
               lastPeriod = taskInfo->deadline;
            }
            taskInfo->priority = prio;
         }
      }
   }
   return 0;
}

void TaskLauncher::createTasks()
{
   #if VERBOSE_INFO
   cout << endl << "CREATING TASKS : " << endl;
   #endif
   //for (auto taskInfo = taskSetInfos.rtTIs.begin(); taskInfo != taskSetInfos.rtTIs.end(); ++taskInfo)
   for (auto& taskInfo : taskSetInfos.rtTIs)
   {
      taskInfo.task = new RT_TASK;
      #if VERBOSE_INFO
      cout << "Creating Task " << taskInfo.name << "." << endl;
      #endif

      if(rt_task_create(taskInfo.task, taskInfo.name, 0, taskInfo.priority, 0) < 0)
      {
         printf("Failed to create task %s\n",taskInfo.name);
      }
      else
      {
         //Periodicity
         int ret = 0;
         if ((ret = rt_task_set_periodic(taskInfo.task, TM_NOW, taskInfo.deadline)))
            cout << "Set_Period Error : " << ret << " ." << endl;
         rt_task_affinity(taskInfo.task, taskInfo.affinity, 0);
         if (schedPolicy == SCHED_RR)
         { //#if defined SCHED_POLICY  &&  SCHED_POLICY == SCHED_RR
            if ((ret = rt_task_slice(taskInfo.task, RR_SLICE_TIME)))
               cout << "Slice Error : " << ret << " ." << endl;
         } //#endif
         #if defined SCHED_POLICY  &&  SCHED_POLICY == SCHED_FIFO
            // FIFO.
         #endif
         if ((ret = rt_task_set_priority(taskInfo.task, taskInfo.priority)))
            cout << "Set_Priority Error : " << ret << " ." << endl;
/*
         RT_TASK_INFO curtaskinfo;
         rt_task_inquire(taskInfo->task, &curtaskinfo);

         struct sched_attr para;
         para.sched_policy = SCHED_POLICY;
         para.sched_flags= 0;
         //para.sched_runtime = taskInfo.deadline;;
         //para.sched_deadline = taskInfo.deadline;
         para.sched_period = taskInfo->deadline;
         para.sched_priority = taskInfo->priority;
         para.size=sizeof(sched_attr);
         rt_task_inquire(taskInfo->task, &curtaskinfo);
         if( sched_setattr(curtaskinfo.pid, &para, 0) != 0)
         {
            fprintf(stderr,"error setting scheduler ... are you root? : %d \n", errno);
            exit(errno);
         }
*/
      }

   }
}

void TaskLauncher::runTasks()
{
   #if VERBOSE_INFO
   cout << endl << "STARTING TASKS : " << endl;
   #endif

   for (auto& taskInfo : taskSetInfos.rtTIs)
   {
      TaskDataLogger* dlog = new TaskDataLogger(&taskInfo);
      taskRTInfo* _taskRTInfo = new taskRTInfo;
      _taskRTInfo->taskLog = dlog;
      _taskRTInfo->rtTI = &taskInfo;
      printInquireInfo(taskInfo.task);
      if( rt_task_start(taskInfo.task, TaskMain, _taskRTInfo))
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
   rt_task_affinity(&mcAgent, 3, 0);

   //  systemRTInfo ch_taks ;
   rt_task_start(&mcAgent, RunmcAgentMain, &taskSetInfos);

}

void TaskLauncher::rt_task_affinity (RT_TASK* task, int _aff, int mode)
{ // mode 0 : replace | mode 1 : add | mode -1 : remove
  cpu_set_t mask;
  if (mode == 0) { CPU_ZERO(&mask); CPU_SET(_aff, &mask); }
  else if (mode > 1) CPU_SET(_aff, &mask);
  else if (mode < -1) CPU_CLR(_aff, &mask);

  RT_TASK_INFO curtaskinfo;
  rt_task_inquire(task, &curtaskinfo);

  if (int ret = rt_task_set_affinity(task, &mask))
  {
    cout << "Error while setting (" << mode << ") affinity for task "
         << curtaskinfo.name << " to CPU " << _aff << ": " <<  ret << "."<< endl;
  }
    #if VERBOSE_ASK
    if (mode == 0) cout << "Switched affinity for task " << curtaskinfo.name << " = CPU " << _aff << endl;
    else if (mode == 1) cout << "Added affinity for task " << curtaskinfo.name << " +CPU " << _aff << endl;
    else if (mode == -1) cout << "Removed affinity for task " << curtaskinfo.name << " -CPU " << _aff << endl;
    #endif
}

void TaskLauncher::stopTasks(bool val)
{

}

void TaskLauncher::saveData(string file)
{
   std::ofstream myFile;
   myFile.open (file);    // TO APPEND :  //,ios_base::app);
   myFile << "timestamp ; name ; ID ; HRT ; deadline ; duration ; affinity \n";
   myFile.close();
   for (auto& taskLog : tasksLogsList)
   {
      taskLog->saveData(file);
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

#include "tools.h"
#include "macroTask.h"
//#include <utmpx.h>    // Pour fonction getcpu()
#include <spawn.h>
#include <sys/wait.h>
#include <fcntl.h>

#define EXECTIME   2e8   // execution time in ns
#define SPINTIME   1e7   // spin time in ns


MacroTask::MacroTask(taskRTInfo* _taskRTInfo, bool MoCo)
{
   properties = _taskRTInfo->rtTI;
   dataLogs = _taskRTInfo->taskLog;
   printTaskInfo(_taskRTInfo->rtTI);

   printInquireInfo(_taskRTInfo->rtTI->task);
   /* Du vieux début...
   RT_TASK_INFO curtaskinfo;
   rt_task_inquire(NULL, &curtaskinfo);
   print_affinity(curtaskinfo.pid);
   #if VERBOSE_OTHER
   cout << "I am task : " << curtaskinfo.name << " of priority " << curtaskinfo.prio << endl;
   #endif
   */
   chain = std::string(properties->path_task) + " " + properties->arguments;
   //cout << "Command for this task is : " << chain << " ." << endl;

   *stdIn = '\0'; *stdOut = '\0'; argv[0] = &properties->name[0u];
   std::istringstream iss(properties->arguments);
   string token;
   int i = 1, nextStr = 0;
   while (getline(iss, token, ' '))
   {
      if (token == "<")
         nextStr = 1;
      else if (token == ">")
         nextStr = 2;
      else
      {
         if (nextStr == 1)  stdIn = &reduce(token)[0u];
         else if (nextStr == 2)  stdOut = &reduce(token)[0u];
         else
         {
            argv[i] = &reduce(token)[0u];
            i++;
         }
         nextStr = 0;
      }
   }

   //cmd = &properties->arguments[0u];

   msg.task    = properties->task;
   msg.ID      = properties->id;
   msg.time    = 0;
   msg.isExecuted = 0;

   MoCoIsAlive = MoCo;
   priority = properties->priority;
/* Ajout subscribe au mutex pour chopper l'ordonnancement sur le coeur
   int cpu = sched_getcpu();
   char* name = &("mutCore" + std::to_string(cpu))[0];
   rt_mutex_bind(&mutex, name, 0);
   cout << "Task " << properties->name << " binded to mutex " << name << endl;
*/
}

int MacroTask::before()
{
//   rt_mutex_acquire(&mutex, TM_INFINITE);
   rt_task_set_priority(NULL, priority+1);
   msg.time = dataLogs->logStart();
   msg.isExecuted = 0;
   if(MoCoIsAlive && (rt_buffer_write(&bf , &msg , sizeof(monitoringMsg) , 100000) < 0))
   {
      //MoCoIsAlive = 0;
      RT_BUFFER_INFO infos;
      rt_buffer_inquire(&bf, &infos);
      std::cerr << properties->name << " : failed to write BEFORE monitoring message to buffer.\n"
          << infos.availmem << " / " << infos.totalmem << " available on buffer " << infos.name << " " << infos.owaiters << " waiting too." << endl;
   }
   return 0;
}

void MacroTask::proceed()
{
      // let the task run RUNTIME ns in steps of SPINTIME ns
      //char* cmd;
      // if (std::string path(properties->path_task) != "/null/")  {
      //char* cmd = &properties->arguments[0u];
      //cout << properties->name << " : " << chain << endl;
      //pid_t cpid;
      if (vfork() == 0)
      {
         if (*stdIn != '\0')
         {
            int fdOut = open(stdOut, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
            dup2(fdOut, 1);
            close(fdOut);
         }
         if (*stdOut != '\0')
         {
         int fdIn = open(stdIn, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
         dup2(fdIn, 0);
         close(fdIn);
         }

         execv(properties->path_task, argv);
      }
      else
      {
         wait(NULL);
      }

      //system(&chain[0u]);
//    }
//    else cout << properties->name <<"Oups, no valid path found !" << endl;
}

int MacroTask::after()
{
  #if VERBOSE_OTHER
  rt_printf("End Task  : %s\n",properties->name);
  #endif
  msg.time = dataLogs->logExec();
  msg.isExecuted = 1;
  if(MoCoIsAlive && (rt_buffer_write(&bf , &msg , sizeof(monitoringMsg) , 100000) < 0))
  {
     //MoCoIsAlive = 0;
     RT_BUFFER_INFO infos;
     rt_buffer_inquire(&bf, &infos);
     std::cerr << properties->name << " : failed to write AFTER monitoring message to buffer.\n"
         << infos.availmem << " / " << infos.totalmem << " available on buffer " << infos.name << " " << infos.owaiters << " waiting too." << endl;
  }
  //ChaineInfo_Struct.Wcet_update() ;
  //ChaineInfo_Struct.Exectime.Update() ;
//  rt_mutex_release(&mutex);
   rt_task_set_priority(NULL, priority);
  return 0;
}

void MacroTask::executeRun()
  {
     if (MoCoIsAlive)
     {
        if( rt_buffer_bind (&bf , "/monitoringTopic", 100000) < 0)
        {
          rt_buffer_delete(&bf);
          rt_printf("%s\n","Failed to link to Monitoring Buffer");
          MoCoIsAlive = 0;
          //exit(-1);
        }
     }
  //cout << "Running..." << endl;
    while (1)
    {
      //cout << "Task" << properties->name << " working." << endl;
      before(); // Check if execution allowed
      proceed();  // execute task
      after();  // Inform of execution time for the mcAgent

      rt_task_wait_period(NULL);

    }

}

////////////////////////////////////////////////////////////////////////////////////////


int MacroTask::before_besteff()
{
//  rt_mutex_acquire(&mutex, TM_INFINITE);
   rt_task_set_priority(NULL, priority+1);
  dataLogs->logStart();
  unsigned int flag;
  rt_event_wait(&event, sizeof(flag), &flag ,	EV_PRIO,TM_NONBLOCK) 	;
  //cout << "Task BE " << properties->name << " not executed." << endl;
  return 0;
}

int MacroTask::after_besteff()
{
    dataLogs->logExec();
    #if VERBOSE_OTHER
    rt_printf("End Task  : %s\n",properties->name);
    #endif
//   rt_mutex_release(&mutex);
   rt_task_set_priority(NULL, priority);
  return 0;
}


void MacroTask::executeRun_besteffort()
{
  //cout << "Running..." << endl;

/*   mutex.lock();
     properties->max_runtime =0;
     properties->min_runtime =1e9;
     properties->out_deadline=0;
     properties->num_of_times=0;
     Somme =0;
     properties->average_runtime =0;
     runtime= 0;
    mutex.unlock();
*/
    while (1)
    {
      before_besteff(); // Check if execution allowed
      proceed();  // execute task
      after_besteff();  // Inform of execution time for the mcAgent
      rt_task_wait_period(NULL);

    }
}

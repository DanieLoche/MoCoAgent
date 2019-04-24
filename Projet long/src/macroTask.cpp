#include "macroTask.h"

#define EXECTIME   2e8   // execution time in ns
#define SPINTIME   1e7   // spin time in ns


MacroTask::MacroTask(taskRTInfo* _taskRTInfo)
{
   properties = _taskRTInfo->rtTI;
   dataLogs = _taskRTInfo->taskLog;
   RT_TASK_INFO curtaskinfo;
   rt_task_inquire(NULL, &curtaskinfo);
   print_affinity(curtaskinfo.pid);
   #if VERBOSE_OTHER
   cout << "I am task : " << curtaskinfo.name << " of priority " << curtaskinfo.prio << endl;
   #endif

   msg.task    = properties->task;
   msg.ID      = properties->id;
   msg.time    = 0;
   msg.isExecuted = 0;

   MoCoIsAlive = 1;

  printTaskInfo(_taskRTInfo->rtTI);
}

int MacroTask::before()
{
  msg.time = dataLogs->logStart();
  msg.isExecuted =0;
  if(MoCoIsAlive && (rt_buffer_write(&bf , &msg , sizeof(monitoringMsg) , 100000) < 0))
  {
     //MoCoIsAlive = 0;
     rt_printf("[%s] : failed to write BEFORE monitoring message to buffer.\n",properties->name);
  }

  return 0;
}

void MacroTask::proceed()
{
      // let the task run RUNTIME ns in steps of SPINTIME ns
      char* cmd;
//      if (std::string path(properties->path_task) != "/null/")  {
         cmd = &properties->path_task[0u];
         system(cmd);
//       }
//       else cout << properties->name <<"Oups, no valid path found !" << endl;
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
     MoCoIsAlive = 0;
     rt_printf("[%s] : failed to write AFTER monitoring message to buffer.\n",properties->name);
  }
  //ChaineInfo_Struct.Wcet_update() ;
  //ChaineInfo_Struct.Exectime.Update() ;

  return 0;
}

void MacroTask::executeRun()
  {
    if( rt_buffer_bind (&bf , "/monitoringTopic", 100000) < 0)
    {
      rt_buffer_delete(&bf);
      rt_printf("%s\n","Failed to link to Monitoring Buffer");
      MoCoIsAlive = 0;
      //exit(-1);
    }
  //cout << "Running..." << endl;
    while (1)
    {
      //cout << "Task" << properties->name << " working." << endl;
      before(); // Check if execution allowed
      proceed();  // execute task
      after();  // Inform of execution time for the mcAgent

      #if defined SCHED_POLICY  &&  SCHED_POLICY == SCHED_RR
        //cout << "POLICY IS RR" << endl;
        rt_task_wait_period(NULL);
      #endif
      #if defined SCHED_POLICY  &&  SCHED_POLICY == SCHED_FIFO
        //cout << "POLICY IS FIFO" << endl;
        rt_task_yield();
      #endif
    }
}

////////////////////////////////////////////////////////////////////////////////////////


int MacroTask::before_besteff()
{
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
      #if defined SCHED_POLICY  &&  SCHED_POLICY == SCHED_RR
        //cout << "POLICY IS RR" << endl;
        rt_task_wait_period(NULL);
      #endif
      #if defined SCHED_POLICY  &&  SCHED_POLICY == SCHED_FIFO
        //cout << "POLICY IS FIFO" << endl;
        rt_task_yield();
      #endif
    }
}

#include "macroTask.h"
#define EXECTIME   2e8   // execution time in ns
#define SPINTIME   1e7   // spin time in ns


extern void print_affinity(pid_t _pid);
extern void printTaskInfo(rtTaskInfosStruct* task);

extern void print_affinity(pid_t _pid);
extern void printTaskInfo(rtTaskInfosStruct* task);

MacroTask::MacroTask()
{

}

int MacroTask::before()
{
  monitoringMsg msg ;
  msg.task = properties.task;
  msg.startTime= rt_timer_read();
  msg.isExecuted =0;
  int ret = rt_buffer_write(&bf , &msg , sizeof(monitoringMsg) , 50000);
  if( 0 > ret)
  {
     printf("fail write : %s\n",properties.name);
  }



  return 0;
}

void MacroTask::proceed(){

      // let the task run RUNTIME ns in steps of SPINTIME ns
      starttime = rt_timer_read();
      char* cmd;
      if (task_path != "/null/")
       {
         cmd = &task_path[0u];
         system(cmd);

       }
       else cout << properties.name <<"Oups, no valid path found !" << endl;
       printf("End Task  : %s\n",properties.name);
     endtime = rt_timer_read();
}




int MacroTask::after()
{
  runtime =  (endtime - starttime)  ;

   Somme += runtime;
   cpt += 1;
   properties.average_runtime = Somme/cpt;
   properties.max_runtime = std::max(runtime,properties.max_runtime );
   properties.min_runtime = std::min(runtime,properties.min_runtime );
   properties.num_of_times=cpt;

  if(runtime <= properties.deadline ){

    printf("[ \033[1;32mPERFECT\033[0m ] Task : %s executed within deadline with execution time of %f ms\n",properties.name,runtime/1e6);
  }else{
    printf("[  \033[1;31mERROR\033[0m  ] Task : %s executed out of deadline with execution time of %f ms\n",properties.name,runtime/1e6);
    properties.out_deadline += 1;
  }
  printf("End Task  : %s\n",properties.name);

  monitoringMsg msg ;
  msg.task= properties.task;
  msg.endTime= endtime;
  msg.isExecuted = 1;
  int ret = rt_buffer_write(&bf , &msg , sizeof(monitoringMsg) , 50000);
  if( 0 > ret)
  {
     printf("fail write : %s\n",properties.name);
  }
  //ChaineInfo_Struct.Wcet_update() ;
  //ChaineInfo_Struct.Exectime.Update() ;

  return 0;
}

void MacroTask::executeRun(RT_SEM* mysync)
{
  //cout << "Running..." << endl;
    int ret;
    mutex.lock();
    properties.max_runtime =0;
    properties.min_runtime =1e9;
    properties.out_deadline=0;
    properties.num_of_times=0;
    Somme =0;
    cout << "path 1:" << properties.path << "." << endl;
    task_path =properties.path ;
    cout << "path 2:" << task_path << "." << endl;
    properties.average_runtime =0;
    runtime= 0;
    cpt =0;
    mutex.unlock();

    rt_sem_p(mysync,TM_INFINITE);

    ret =  rt_buffer_bind (&bf , "/monitoringTopic" ,50000);
    if( 0 > ret)
    {
      rt_buffer_delete(&bf);
      printf("%s\n","fail creat");
    }

    while (1) {

      before(); // Check if execution allowed

      proceed();  // execute task

      after();  // Inform of execution time for the mcAgent



      rt_task_wait_period(NULL);

    }
}

////////////////////////////////////////////////////////////////////////////////////////


int MacroTask::before_besteff()
{
  unsigned int flag;
  rt_event_wait(&event,sizeof(flag), &flag ,	EV_PRIO,TM_NONBLOCK) 	;
  return 0;
}

int MacroTask::after_besteff()
{
  runtime =  (endtime - starttime)  ;

   Somme += runtime;
   cpt += 1;
   properties.average_runtime = Somme/cpt;
   properties.max_runtime = std::max(runtime,properties.max_runtime );
   properties.min_runtime = std::min(runtime,properties.min_runtime );
   properties.num_of_times=cpt;

  if(runtime <= properties.deadline ){

    printf("[ \033[1;32mPERFECT\033[0m ] Task : %s executed within deadline with execution time of %f ms\n",properties.name,runtime/1e6);
  }else{
    printf("[  \033[1;31mERROR\033[0m  ] Task : %s executed out of deadline with execution time of %f ms\n",properties.name,runtime/1e6);
    properties.out_deadline += 1;
  }
  printf("End Task  : %s\n",properties.name);

  return 0;
}


void MacroTask::executeRun_besteffort(RT_SEM* mysync)
{
  //cout << "Running..." << endl;

    mutex.lock();
    properties.max_runtime =0;
    properties.min_runtime =1e9;
    properties.out_deadline=0;
    properties.num_of_times=0;
    Somme =0;
    cout << "path 1:" << properties.path << "." << endl;
    task_path =properties.path ;
    cout << "path 2:" << task_path << "." << endl;
    properties.average_runtime =0;
    runtime= 0;
    cpt =0;
    mutex.unlock();

    rt_sem_p(mysync,TM_INFINITE);
    
    int ret;
    ret = rt_event_bind( &event,"/modeChangeTopic",500000 );
    if( 0 > ret)
    {
      rt_event_delete(&event);
      printf("fail bind event %s\n",properties.name);
    }

    while (1) {

      before_besteff(); // Check if execution allowed

      proceed();  // execute task

      after_besteff();  // Inform of execution time for the mcAgent



      rt_task_wait_period(NULL);

    }
}

#include "macroTask.h"
#define EXECTIME   2e8   // execution time in ns
#define SPINTIME   1e7   // spin time in ns


extern void print_affinity(pid_t _pid);
extern void printTaskInfo(rtTaskInfosStruct* task);

MacroTask::MacroTask()
{

}

int MacroTask::before()
{
  /*  if ((!properties->isHardRealTime) && ( Emergency )) {
    rt_sem_p(emergency,TM_INFINITE);
  }
  */
//  printTaskInfo(properties);
  print_affinity(0);
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
       else cout << properties->name <<"Oups, no valid path found !" << endl;
       printf("End Task  : %s\n",properties->name);
     endtime = rt_timer_read();
}




int MacroTask::after()
{
  runtime =  (endtime - starttime)  ;

   Somme += runtime;
   cpt += 1;
   properties->average_runtime = Somme/cpt;
   properties->max_runtime = std::max(runtime,properties->max_runtime );
   properties->min_runtime = std::min(runtime,properties->min_runtime );
   properties->num_of_times=cpt;

  if(runtime <= properties->deadline ){

    printf("[ \033[1;32mPERFECT\033[0m ] Task : %s executed within deadline with execution time of %f ms\n",properties->name,runtime/1e6);
  }else{
    printf("[  \033[1;31mERROR\033[0m  ] Task : %s executed out of deadline with execution time of %f ms\n",properties->name,runtime/1e6);
    properties->out_deadline += 1;
  }
  printf("End Task  : %s\n",properties->name);

  //ChaineInfo_Struct.Wcet_update() ;
  //ChaineInfo_Struct.Exectime.Update() ;

  return 0;
}

void MacroTask::executeRun(RT_SEM* mysync,RT_BUFFER* bf)
{
  //cout << "Running..." << endl;
    int ret;
    mutex.lock();

    properties->max_runtime =0;
    properties->min_runtime =1e9;
    properties->out_deadline=0;
    properties->num_of_times=0;
    Somme =0;
    cout << "path 1:" << properties->path << "." << endl;
    task_path =properties->path ;
    cout << "path 2:" << task_path << "." << endl;
    properties->average_runtime =0;
    runtime= 0;
    cpt =0;
    mutex.unlock();

    rt_sem_p(mysync,TM_INFINITE);

    while (1) {

      before(); // Check if execution allowed

      proceed();  // execute task

      after();  // Inform of execution time for the mcAgent

      char* msg ;

      ret = rt_buffer_write(&bf , msg , 200 , 50000);
      printf(" write buf ret : %d\n",ret);

      rt_task_wait_period(NULL);

    }
}

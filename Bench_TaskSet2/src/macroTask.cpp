#include "macroTask.h"
#define EXECTIME   2e8   // execution time in ns
#define SPINTIME   1e7   // spin time in ns
//#define PRINT

extern void print_affinity(pid_t _pid);
extern void printTaskInfo(rtTaskInfosStruct* task);

MacroTask::MacroTask()
{

}

int MacroTask::before()
{
  starttime = rt_timer_read();
  print_affinity(0);
  return 0;
}

void MacroTask::proceed(){

      //getrusage(RUSAGE_SELF,&usage);

      char* cmd;
      if (properties->path != "/null/")
       {
         cmd = &properties->path[0u];
         system(cmd);
       }
       else cout << properties->name <<"Oups, no valid path found !" << endl;
       //getrusage(RUSAGE_SELF,&usage);



}




int MacroTask::after()
{
   endtime = rt_timer_read();
   runtime =  (endtime - starttime)  ;
   Somme += runtime;
   cpt += 1;
   properties->average_runtime = Somme/cpt;
   properties->max_runtime = std::max(runtime,properties->max_runtime );
   properties->min_runtime = std::min(runtime,properties->min_runtime );
   properties->num_of_times=cpt;
  if(runtime <= properties->deadline ){
    #ifdef PRINT
    printf("[ \033[1;32mPERFECT\033[0m ] Task : %s executed within deadline with execution time of %f ms\n",properties->name,runtime/1e6);
    #endif
  }else{
    #ifdef PRINT
    printf("[  \033[1;31mERROR\033[0m  ] Task : %s executed out of deadline with execution time of %f ms\n",properties->name,runtime/1e6);
    #endif
    properties->out_deadline += 1;
  }
  #ifdef PRINT
  printf("End Task  : %s\n",properties->name);
  #endif
  properties->Exectued = 1;
  //ChaineInfo_Struct.Wcet_update() ;
  //ChaineInfo_Struct.Exectime.Update() ;
  return 0;
}





void MacroTask::executeRun()
{
   properties->max_runtime =0;
  properties->min_runtime =1e9;
  properties->out_deadline=0;
  properties->num_of_times=0;
  Somme =0;
  //cout << "path :" << properties.path << "." << endl;
  properties->average_runtime =0;
  runtime= 0;
  cpt =0;

  //cout << "Running..." << endl;

      before(); // Check if execution allowed

      proceed();  // execute task

      after();  // Inform of execution time for the mcAgent

      //rt_task_wait_period(NULL);


}

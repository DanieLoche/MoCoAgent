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
  print_affinity(0);
  return 0;
}

void MacroTask::proceed(RT_SEM* mysync)
{
  string cmd;
  RTIME starttime, runtime,endtime,time,Somme;
  properties->max_runtime =0;
  properties->min_runtime =1e9;
  properties->out_deadline=0;
  Somme =0;
  //cout << "path :" << properties.path << "." << endl;
  properties->average_runtime =0;
  runtime= 0;
  int num =1;
  rt_sem_p(mysync,TM_INFINITE);

  while (1) {
      // let the task run RUNTIME ns in steps of SPINTIME ns
      time = 0;
     starttime = rt_timer_read();
/*
     while(time < EXECTIME) {
       rt_timer_spin(SPINTIME);  // spin cpu doing nothing
       time = time + SPINTIME;
       cout<<"Running Task  :"<< properties->name <<"at time : "<<time<<endl ;
       //printf("Running Task  : %s  at time : %lld \n",properties->name,time);
     }
     printf("End Task  : %s\n",properties->name);
*/
      if (properties->path != "/null/")
       {
         cmd = properties->path;
         cmd.append(" ");
         cmd.append(properties->name);
         system(cmd.c_str());
       }
       else cout << properties->name <<"Oups, no valid path found !" << endl;

     endtime = rt_timer_read();

     runtime =  (endtime - starttime)  ;

     Somme += runtime;
     properties->max_runtime = std::max(runtime,properties->max_runtime );

     properties->min_runtime = std::min(runtime,properties->min_runtime );
     properties->average_runtime =Somme/num;
     num += 1;

     if(runtime <= properties->deadline ){
       printf("Task  : %s within deadline\n",properties->name);
     }else{
       printf("Task  : %s out of  deadline\n",properties->name);
       properties->out_deadline += 1;
     }

     rt_task_wait_period(NULL);

  }
    printf("End Task  : %s\n",properties->name);
/*
    while(time < EXECTIME) {
      rt_timer_spin(SPINTIME);  // spin cpu doing nothing
      time = time + SPINTIME;
      //cout<<"Running Task  :"<< properties.name <<"at time : "<<runtime<<endl ;
      printf("Running Task  : %s  at time : %lld \n",properties->name,time);
    }
    printf("End Task  : %s\n",properties->name);

*/
      /*  if (properties.path != "/null/")
        {
          cmd = &properties.path[0u];
          system(cmd);
        }
        else cout << properties.name <<"Oups, no valid path found !" << endl;

        */
}

int MacroTask::after()
{

  return 0;
}

void MacroTask::executeRun(RT_SEM* mysync)
{
  //cout << "Running..." << endl;
  //before(); // Check if execution allowed

  proceed(mysync);  // execute task

  if (properties->isHardRealTime)
    after();  // Inform of execution time for the mcAgent
}

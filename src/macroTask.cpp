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
  char* cmd;
  RTIME  runtime;
  cout << "path :" << properties.path << "." << endl;

  rt_sem_p(mysync,TM_INFINITE);
  while (1) {
           // let the task run RUNTIME ns in steps of SPINTIME ns
           runtime = 0;
           while(runtime < EXECTIME) {
             rt_timer_spin(SPINTIME);  // spin cpu doing nothing
             runtime = runtime + SPINTIME;
             cout<<"Running Task  :"<< properties.name <<"at time : "<< runtime<<endl ;
           }
           rt_task_wait_period(NULL);
           cout<< properties.name<<rt_timer_read()<<endl;

 }

   cout<<"End Task  :"<< properties.name <<endl;


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
  cout << "Running..." << endl;
  before(); // Check if execution allowed

  proceed(mysync);  // execute task

  if (properties.isHardRealTime)
    after();  // Inform of execution time for the mcAgent
}

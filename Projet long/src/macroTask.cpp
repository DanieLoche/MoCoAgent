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
   /* Du vieux débug...
   RT_TASK_INFO curtaskinfo;
   rt_task_inquire(NULL, &curtaskinfo);
   print_affinity(curtaskinfo.pid);
   #if VERBOSE_OTHER
   cout << "I am task : " << curtaskinfo.name << " of priority " << curtaskinfo.prio << endl;
   #endif
   */
   //chain = std::string(properties->path_task) + " " + properties->arguments;
   //cout << "Command for this task is : " << chain << " ." << endl;
   parseParameters( );
        if (strcmp(properties->path_task, "basicmath_small")) proceed_function = basicmath_small;
   else if (strcmp(properties->path_task, "basicmath_large")) proceed_function = basicmath_large;
   else if (strcmp(properties->path_task, "bitcnts"))         proceed_function = bitcount_func;
   else if (strcmp(properties->path_task, "qsort_small"))     proceed_function = qsort_small;
   else if (strcmp(properties->path_task, "qsort_large"))     proceed_function = qsort_large;
   else if (strcmp(properties->path_task, "susan_bin"))       proceed_function = susan;
   else if (strcmp(properties->path_task, "cjpeg"))           proceed_function = cjpeg_func;
   else if (strcmp(properties->path_task, "djpeg"))           proceed_function = djpeg_func;
   else if (strcmp(properties->path_task, "lout"))            proceed_function = typeset_func;
   else if (strcmp(properties->path_task, "dijkstra_small"))  proceed_function = dijkstra_small;
   else if (strcmp(properties->path_task, "dijkstra_large"))  proceed_function = dijkstra_large;
   else if (strcmp(properties->path_task, "patricia_bin"))    proceed_function = patricia;
   else if (strcmp(properties->path_task, "search_large"))    proceed_function = stringsearch_small;
   else if (strcmp(properties->path_task, "search_small"))    proceed_function = stringsearch_large;
   else if (strcmp(properties->path_task, "bf"))              proceed_function = blowfish;
   else if (strcmp(properties->path_task, "rijndael"))        proceed_function = rijndael;
   else if (strcmp(properties->path_task, "sha_bin"))         proceed_function = sha;
   else if (strcmp(properties->path_task, "adpcm_rawcaudio")) proceed_function = rawcaudio;
   else if (strcmp(properties->path_task, "adpcm_rawdaudio")) proceed_function = rawdaudio;
   else if (strcmp(properties->path_task, "crc"))             proceed_function = crc;
   else if (strcmp(properties->path_task, "fft"))             proceed_function = fft;
   else if (strcmp(properties->path_task, "toast"))           proceed_function = gsm_func;
   else if (strcmp(properties->path_task, "untoast"))         proceed_function = gsm_func;

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

void MacroTask::parseParameters()
{
      //cout << "[ " << properties->name << " ] : " << "Started parsing params." << endl;
      stdIn[0] = '\0'; stdOut[0] = '\0';
      argv.push_back(properties->name); //argv[0] = properties->name;

      std::istringstream iss( properties->arguments);
      string token;
      int nextStr = 0;
      //argc = 1;
      while (getline(iss, token, ' '))
      {
         token = reduce(token);
         //cout << "[ " << properties->name << " ] : " << "Managing token [" << token << "]" << endl;
         if (token == "<")
            nextStr = 1;
         else if (token == ">")
            nextStr = 2;
         else
         {
            if (nextStr == 1)
            {
               token.copy(stdIn, token.size());
               stdIn[token.size()] = '\0';
               #if VERBOSE_OTHER
               cout << "[ " << properties->name << " ] : " << "stdIn = " << stdIn << "." << endl;
               #endif
            }
            else if (nextStr == 2)
            {
               token.copy(stdOut, token.size());
               stdOut[token.size()] = '\0';
               #if VERBOSE_OTHER
               cout << "[ " << properties->name << " ] : " << "stdOut = " << stdOut << "." << endl;
               #endif
            }
            else
            {
               char *arg = new char[token.size() +1];  // +1
               copy(token.begin(), token.end(), arg);
               arg[token.size()]= '\0';
               #if VERBOSE_OTHER
               cout << "token : [" << token << "] (" << token.size() << ") copied to [" << arg << "] (" << strlen(arg) << ")." << endl;
               #endif
               argv.push_back(arg);
               //argc++;
            }
            nextStr = 0;
         }
      }
      argv.push_back(0);
      //token.copy(argv[i], token.size()); // arguments list must end with a null.
      #if VERBOSE_OTHER
      int i = 1;
      i = 0;
      for (auto arg : argv)
      {
         string toPrint;
         if (arg==NULL)toPrint = "null";
         else toPrint = arg;
         cout << "Arg #" << i << " = " << toPrint << " ; ";
         i++;
      }
      cout << endl;
      #endif

      // if (stdIn[0] != '\0')
      // {
      //    #if VERBOSE_OTHER
      //    cout << "Changed Input to : " << stdIn << endl;
      //    #endif
      //    int fdIn = open(stdIn, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
      //    dup2(fdIn, 0);
      //    close(fdIn);
      // }
      // #if VERBOSE_OTHER
      // else cout << "Unchanged Input." << endl;
      // #endif
      //
      // if (stdOut[0] != '\0')
      // {
      //    #if VERBOSE_OTHER
      //    cout << "Changed Output to : " << stdOut << endl;
      //    #endif
      //    int fdOut = open(stdOut, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
      //    dup2(fdOut, 1);
      //    close(fdOut);
      // }
      // #if VERBOSE_OTHER
      // else
      // cout << "Unchanged Output." << endl;
      // #endif

}

int MacroTask::before()
{
   //   rt_mutex_acquire(&mutex, TM_INFINITE);
   #if VERBOSE_OTHER
   cout << "[ " << properties->name << " ] : " << "Executing Before." << endl;
   #endif
   //rt_task_set_priority(NULL, priority+1);
   msg.time = dataLogs->logStart();
   msg.isExecuted = 0;
   if(MoCoIsAlive && (rt_buffer_write(&bf , &msg , sizeof(monitoringMsg) , 1e6) < 0))
   {
      //MoCoIsAlive = 0;
      RT_BUFFER_INFO infos;
      rt_buffer_inquire(&bf, &infos);
      cerr << properties->name << " : failed to write BEFORE monitoring message to buffer." << "(Moco : " << MoCoIsAlive << ")" << endl
          << infos.availmem << " / " << infos.totalmem << " available on buffer " << infos.name << " " << infos.owaiters << " waiting too." << endl;
   }
   return 0;
}

void MacroTask::proceed()
{
      #if VERBOSE_OTHER
      cout << "[ " << properties->name << " ] : "<< "Executing Proceed." << endl;
      #endif

      proceed_function(argv.size() - 1, &argv[0]);  // -1 : no need for last element "NULL".
      // if (std::string path(properties->path_task) != "/null/")  {
      //cout << properties->name << " : " << chain << endl;
/*
      if (vfork() == 0)
      {
          if (stdIn[0] != '\0')
          {
             int fdIn = open(stdIn, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
             dup2(fdIn, 0);
             close(fdIn);
          }

          if (stdOut[0] != '\0')
          {
             int fdOut = open(stdOut, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
             dup2(fdOut, 1);
             close(fdOut);
          }
         execv(properties->path_task, &argv[0]);
         _exit(0);
      }
      else
      {
         wait(NULL);
         #if VERBOSE_OTHER
         cout << "[ " << properties->name << " ] : "<< "End of Proceed." << endl;
         #endif
      }
*/
      //system(&chain[0u]);
//    }
//    else cout << properties->name <<"Oups, no valid path found !" << endl;
}

int MacroTask::after()
{
   #if VERBOSE_OTHER
   cout << "[ " << properties->name << " ] : "<< "Executing After." << endl;
   #endif
   msg.time = dataLogs->logExec();
   msg.isExecuted = 1;
   if(MoCoIsAlive && (rt_buffer_write(&bf , &msg , sizeof(monitoringMsg) , 1e6) < 0))
   {
      //MoCoIsAlive = 0;
      RT_BUFFER_INFO infos;
      rt_buffer_inquire(&bf, &infos);
      cerr << properties->name << " : failed to write AFTER monitoring message to buffer." << "(Moco : " << MoCoIsAlive << ")" << endl
         << infos.availmem << " / " << infos.totalmem << " available on buffer " << infos.name << " " << infos.owaiters << " waiting too." << endl;
   //rt_task_set_priority(NULL, priority);
   }
   //  rt_mutex_release(&mutex);
   return 0;
}

void MacroTask::executeRun()
  {
     if (MoCoIsAlive)
     {
        if( rt_buffer_bind (&bf , "/monitoringTopic", 100000) < 0)
        {
          rt_buffer_delete(&bf);
          cerr << "Failed to link to Monitoring Buffer" << endl;
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

      rt_task_wait_period(&dataLogs->overruns);

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
    cout << "End Task : " << properties->name << endl;
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
      rt_task_wait_period(&dataLogs->overruns);

    }
}

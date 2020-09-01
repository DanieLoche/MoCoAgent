#include "tools.h"
#include "macroTask.h"
//#include <utmpx.h>    // Pour fonction getcpu()
//#include <spawn.h>
//#include <sys/wait.h>
#include <fcntl.h>      // gestion systèmes de fichiers ( dup() )

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
   parseParameters( );
   #if VERBOSE_DEBUG
   cout << "Command for this task is : " << properties->path_task << " . ";
   int i = 0;
   for (auto arg : _argv)
   {
      string toPrint;
      if (arg==NULL)toPrint = "null";
      else toPrint = arg;
      cout << "Arg #" << i << " = " << toPrint << " ; ";
      i++;
   }
   cout << endl;
   #endif

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

void MacroTask::configure(rtTaskInfosStruct taskInfo, int schedPolicy)
{
   int ret = 0;
   //taskInfo.task = new RT_TASK;
   TRY( rt_task_shadow(taskInfo.task, taskInfo.name, taskInfo.priority, T_WARNSW) );
   /*{
      cerr << "[" << taskInfo.name << "] " << "Failed to create Xenomai task." << endl;
      exit(-1);
   }*/
   //else // Configure Xenomai task
   {
      RT_TASK_INFO curtaskinfo;
      rt_task_inquire(taskInfo.task, &curtaskinfo);
      struct sched_param_ex para;
      para.sched_priority = taskInfo.priority;
      /* sched_param
      para.sched_flags= 0;
      para.sched_runtime = taskInfo.periodicity;;
      para.sched_deadline = taskInfo.periodicity;
      para.sched_period = taskInfo->periodicity;
      para.size=sizeof(sched_attr);
      old... */

      if (schedPolicy == SCHED_RM) schedPolicy = SCHED_FIFO;

      if( (ret = sched_setscheduler_ex(curtaskinfo.pid, schedPolicy, &para)) != 0)
      {
         cerr << "[" << taskInfo.name << "] " << "error setting scheduling policy " << schedPolicy << ", Error #" << ret;
         exit(ret);
      }

      if (schedPolicy == SCHED_RR)
      { //#if defined SCHED_POLICY  &&  SCHED_POLICY == SCHED_RR
         if ((ret = rt_task_slice(taskInfo.task, RR_SLICE_TIME)))
         { cerr << "[" << taskInfo.name << "] " << "Slice Error : " << ret << " ." << endl; exit(-4); }
      } //#endif

      //Periodicity
      if ((ret = rt_task_set_periodic(taskInfo.task, TM_NOW, taskInfo.periodicity)))
      {  cerr << "[" << taskInfo.name << "] " << "Set_Period Error : " << ret << " ." << endl; exit(-2); }

      rt_task_affinity(taskInfo.task, taskInfo.affinity, 0);

      if ((ret = rt_task_set_priority(taskInfo.task, taskInfo.priority)))
         { cerr << "[" << taskInfo.name << "] " << "Set_Priority Error : " << ret << " ." << endl; exit(-5); }
      /* Gestion EDF Scheduling
      RT_TASK_INFO curtaskinfo;
      rt_task_inquire(taskInfo->task, &curtaskinfo);

      struct sched_attr para;
      para.sched_policy = SCHED_POLICY;
      para.sched_flags= 0;
      //para.sched_runtime = taskInfo.periodicity;;
      //para.sched_deadline = taskInfo.periodicity;
      para.sched_period = taskInfo->periodicity;
      para.sched_priority = taskInfo->priority;
      para.size=sizeof(sched_attr);
      rt_task_inquire(taskInfo->task, &curtaskinfo);
      if( sched_setattr(curtaskinfo.pid, &para, 0) != 0)
      {
         fprintf(stderr,"error setting scheduler ... are you root? : %d \n", errno);
         exit(errno);
      }
   */

      TRY( rt_task_suspend(NULL) );


   }

}

void MacroTask::parseParameters()
{
      //cout << "[ " << properties->name << " ] : " << "Started parsing params." << endl;
      stdIn[0] = '\0'; stdOut[0] = '\0';
      if      (!strcmp(properties->path_task, "basicmath_small")) proceed_function = basicmath_small;
      else if (!strcmp(properties->path_task, "basicmath_large")) proceed_function = basicmath_large;
      else if (!strcmp(properties->path_task, "bitcnts"))         proceed_function = bitcount_func;
      else if (!strcmp(properties->path_task, "qsort_small"))     proceed_function = qsort_small;
      else if (!strcmp(properties->path_task, "qsort_large"))     proceed_function = qsort_large;
      else if (!strcmp(properties->path_task, "susan_bin"))       proceed_function = susan;
      else if (!strcmp(properties->path_task, "cjpeg"))           proceed_function = cjpeg_func;
      else if (!strcmp(properties->path_task, "djpeg"))           proceed_function = djpeg_func;
      else if (!strcmp(properties->path_task, "lout"))            proceed_function = typeset_func;
      else if (!strcmp(properties->path_task, "dijkstra_small"))  proceed_function = dijkstra_small;
      else if (!strcmp(properties->path_task, "dijkstra_large"))  proceed_function = dijkstra_large;
      else if (!strcmp(properties->path_task, "patricia_bin"))    proceed_function = patricia;
      else if (!strcmp(properties->path_task, "search_large"))    proceed_function = stringsearch_small;
      else if (!strcmp(properties->path_task, "search_small"))    proceed_function = stringsearch_large;
      else if (!strcmp(properties->path_task, "bf"))              proceed_function = blowfish;
      else if (!strcmp(properties->path_task, "rijndael"))        proceed_function = rijndael;
      else if (!strcmp(properties->path_task, "sha_bin"))         proceed_function = sha;
      else if (!strcmp(properties->path_task, "adpcm_rawcaudio")) proceed_function = rawcaudio;
      else if (!strcmp(properties->path_task, "adpcm_rawdaudio")) proceed_function = rawdaudio;
      else if (!strcmp(properties->path_task, "crc"))             proceed_function = crc;
      else if (!strcmp(properties->path_task, "fft"))             proceed_function = fft;
      else if (!strcmp(properties->path_task, "toast"))           proceed_function = gsm_func;
      else if (!strcmp(properties->path_task, "untoast"))         proceed_function = gsm_func;

      _argv.push_back(properties->path_task); //argv[0] = properties->name;


      //new_fd = dup(1);
      properties->arguments = reduce(properties->arguments);
      int chr;
      for (chr = 0; properties->arguments[chr] != '\0'; chr++)
      {
        if (properties->arguments[chr] == '\n' || properties->arguments[chr] == '\r')
          { properties->arguments[chr] = ' '; cout << "/!\\ Strange ASCII char found !!" << endl; }
      }

      std::istringstream iss( properties->arguments);
      //cout << "Managing arguments : [" << properties->arguments << "]" << endl;
      string token;
      int nextStr = 0;
      //_argc = 1;
      while (getline(iss, token, ' '))
      {
         token = reduce(token);
         //cout << "[ " << properties->name << " ] : " << "Managing token [" << token << "]." << endl;
         if (token == "<")
            nextStr = 1;
         else if (token == ">")
            nextStr = 2;
         else if (token != "")
         {
            if (nextStr == 1)
            {
               #if VERBOSE_OTHER
               cout << "[ " << properties->name << " ] : " << "stdIn = " << stdIn << "." << endl;
               #endif
               token.copy(stdIn, token.size());
               stdIn[token.size()] = '\0';
               //inStrm.open(stdIn);
            }
            else if (nextStr == 2)
            {
               #if VERBOSE_OTHER
               cout << "[ " << properties->name << " ] : " << "stdOut = " << stdOut << "." << endl;
               #endif
               token.copy(stdOut, token.size());
               stdOut[token.size()] = '\0';
               //new_fd = open(stdOut, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
               //outStrm.open(stdOut);
            }
            else
            {
               char *arg = new char[token.size() +1];  // +1
               copy(token.begin(), token.end(), arg);
               arg[token.size()]= '\0';
               #if VERBOSE_OTHER
               cout << "token : [" << token << "] (" << token.size() << ") copied to [" << arg << "] (" << strlen(arg) << ")." << endl;
               #endif

               _argv.push_back(arg);
               //argc++;
            }
            nextStr = 0;
         }
      }
      _argv.push_back(0);
      //token.copy(argv[i], token.size()); // arguments list must end with a null.
      #if VERBOSE_OTHER
      int i = 0;
      for (auto arg : _argv)
      {
         string toPrint;
         if (arg==NULL)toPrint = "null";
         else toPrint = arg;
         cout << "Arg #" << i << " = " << toPrint << " ; ";
         i++;
      }
      cout << endl;
      #endif

      if (stdIn[0] != '\0')
      {
         #if VERBOSE_OTHER
         cout << "Changed Input to : " << stdIn << endl;
         #endif
         int fdIn = open(stdIn, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
         dup2(fdIn, 0);
         close(fdIn);
      }
      #if VERBOSE_OTHER
      else cout << "Unchanged Input." << endl;
      #endif


      if (stdOut[0] != '\0')
      {
         #if VERBOSE_OTHER
         cout << "Changed Output to : " << stdOut << endl;
         #endif
         int fdOut = open(stdOut, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
         dup2(fdOut, 1);
         close(fdOut);
      }
      #if VERBOSE_OTHER
      else
      cout << "Unchanged Output." << endl;
      #endif

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

/*
      if (stdIn[0] != '\0')
      {
          std::cin.rdbuf(inStrm.rdbuf()); //redirect std::cin to stdIn!
      }

      if (stdOut[0] != '\0')
      {
          std::cout.rdbuf(outStrm.rdbuf()); //redirect std::cout to stdOut!
      }
*/
      if (stdIn[0] != '\0')
      {
         int fdIn = open(stdIn, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
         dup2(fdIn, 0);
         close(fdIn);
      }

      /* utilisation d'un new_fd
         if (stdOut[0] != '\0')
         {
         fflush(stdout);
         dup2(new_fd, 1);
         }
      */


      int ret = proceed_function(_argv.size()-1, &_argv[0]);  // -1 : no need for last element "NULL".
      if (ret != 0)
      {
        cerr << "["<< properties->name << " ("<< getpid() << ")] - Error during proceed ! [" << ret << "]. Function was : ";
        int i = 1;
        i = 0;
        for (auto arg : _argv)
        {
           string toPrint;
           if (arg==NULL)toPrint = "null";
           else toPrint = arg;
           cerr << "Arg #" << i << " = " << toPrint << " ; ";
           i++;
        }
        cerr << " with (" << _argv.size()-1 << ") elements." << endl;
      }


      //if (stdIn[0] != '\0') std::cin.rdbuf(cinbuf);   //reset to standard input again
      //if (stdOut[0] != '\0') std::cout.rdbuf(coutbuf); //reset to standard output again

      // if (std::string path(properties->path_task) != "/null/")  {
      //cout << properties->name << " : " << chain << endl;

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
   //rt_task_set_priority(NULL, priority+1);
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
   // rt_task_set_priority(NULL, priority);
  return 0;
}


void MacroTask::executeRun_besteffort()
{
  //cout << "Running..." << endl;

    while (1)
    {
      before_besteff(); // Check if execution allowed
      proceed();  // execute task
      after_besteff();  // Inform of execution time for the mcAgent
      rt_task_wait_period(&dataLogs->overruns);

    }
}

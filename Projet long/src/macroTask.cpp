#include "tools.h"
#include "macroTask.h"
//#include <utmpx.h>    // Pour fonction getcpu()
//#include <spawn.h>
//#include <sys/wait.h>
#include <fcntl.h>      // gestion systèmes de fichiers ( dup() )

#define EXECTIME   2e8   // execution time in ns
#define SPINTIME   1e7   // spin time in ns


MacroTask::MacroTask(rtTaskInfosStruct _taskRTInfo, bool MoCo)
{
   prop = _taskRTInfo;
   dataLogs = new TaskDataLogger(&prop);

   printTaskInfo(&_taskRTInfo);

   printInquireInfo(&_task);
   /* Du vieux débug...
   RT_TASK_INFO curtaskinfo;
   rt_task_inquire(NULL, &curtaskinfo);
   print_affinity(curtaskinfo.pid);
   #if VERBOSE_OTHER
   cout << "I am task : " << curtaskinfo.name << " of priority " << curtaskinfo.prio << endl;
   #endif
   */
   //chain = std::string(prop.function) + " " + prop.arguments;
   parseParameters( );
   #if VERBOSE_DEBUG
   cout << "Command for this task is : " << prop.function << " . ";
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

   msg.task    = &_task;
   msg.ID      = prop.id;
   msg.time    = 0;
   msg.isExecuted = 0;

   MoCoIsAlive = MoCo;
/* Ajout subscribe au mutex pour chopper l'ordonnancement sur le coeur
   int cpu = sched_getcpu();
   char* name = &("mutCore" + std::to_string(cpu))[0];
   rt_mutex_bind(&mutex, name, 0);
   cout << "Task " << prop.name << " binded to mutex " << name << endl;
*/
}

void MacroTask::setRTtask( )
{
   int ret = 0;
   ERROR_MNG( rt_task_shadow(&_task, prop.name, prop.priority, T_WARNSW) );

   RT_TASK_INFO curtaskinfo;
   rt_task_inquire(&_task, &curtaskinfo);
   struct sched_param_ex para;
   para.sched_priority = prop.priority;
   /* sched_param
   para.sched_flags= 0;
   para.sched_runtime = taskInfo.periodicity;;
   para.sched_deadline = taskInfo.periodicity;
   para.sched_period = taskInfo->periodicity;
   para.size=sizeof(sched_attr);
   old... */

   if (prop.schedPolicy == SCHED_RM) prop.schedPolicy = SCHED_FIFO;

   if( (ret = sched_setscheduler_ex(curtaskinfo.pid, prop.schedPolicy, &para)) != 0)
   {
      cerr << "[" << prop.name << "] " << "error setting scheduling policy " << prop.schedPolicy << ", Error #" << ret;
      exit(ret);
   }

   if (prop.schedPolicy == SCHED_RR)
   { //#if defined SCHED_POLICY  &&  SCHED_POLICY == SCHED_RR
      if ((ret = rt_task_slice(&_task, RR_SLICE_TIME)))
      { cerr << "[" << prop.name << "] " << "Slice Error : " << ret << " ." << endl; exit(-4); }
   } //#endif

   //Periodicity
   if ((ret = rt_task_set_periodic(&_task, TM_NOW, prop.periodicity)))
   {  cerr << "[" << prop.name << "] " << "Set_Period Error : " << ret << " ." << endl; exit(-2); }

   setAffinity(prop.affinity, 0);

   if ((ret = rt_task_set_priority(&_task, prop.priority)))
      { cerr << "[" << prop.name << "] " << "Set_Priority Error : " << ret << " ." << endl; exit(-5); }
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

   ERROR_MNG( rt_task_suspend(NULL) );

}

void MacroTask::parseParameters()
{
      //cout << "[ " << prop.name << " ] : " << "Started parsing params." << endl;
      stdIn[0] = '\0'; stdOut[0] = '\0';
      if      (!strcmp(prop.function, "basicmath_small")) proceed_function = basicmath_small;
      else if (!strcmp(prop.function, "basicmath_large")) proceed_function = basicmath_large;
      else if (!strcmp(prop.function, "bitcnts"))         proceed_function = bitcount_func;
      else if (!strcmp(prop.function, "qsort_small"))     proceed_function = qsort_small;
      else if (!strcmp(prop.function, "qsort_large"))     proceed_function = qsort_large;
      else if (!strcmp(prop.function, "susan_bin"))       proceed_function = susan;
      else if (!strcmp(prop.function, "cjpeg"))           proceed_function = cjpeg_func;
      else if (!strcmp(prop.function, "djpeg"))           proceed_function = djpeg_func;
      else if (!strcmp(prop.function, "lout"))            proceed_function = typeset_func;
      else if (!strcmp(prop.function, "dijkstra_small"))  proceed_function = dijkstra_small;
      else if (!strcmp(prop.function, "dijkstra_large"))  proceed_function = dijkstra_large;
      else if (!strcmp(prop.function, "patricia_bin"))    proceed_function = patricia;
      else if (!strcmp(prop.function, "search_large"))    proceed_function = stringsearch_small;
      else if (!strcmp(prop.function, "search_small"))    proceed_function = stringsearch_large;
      else if (!strcmp(prop.function, "bf"))              proceed_function = blowfish;
      else if (!strcmp(prop.function, "rijndael"))        proceed_function = rijndael;
      else if (!strcmp(prop.function, "sha_bin"))         proceed_function = sha;
      else if (!strcmp(prop.function, "adpcm_rawcaudio")) proceed_function = rawcaudio;
      else if (!strcmp(prop.function, "adpcm_rawdaudio")) proceed_function = rawdaudio;
      else if (!strcmp(prop.function, "crc"))             proceed_function = crc;
      else if (!strcmp(prop.function, "fft"))             proceed_function = fft;
      else if (!strcmp(prop.function, "toast"))           proceed_function = gsm_func;
      else if (!strcmp(prop.function, "untoast"))         proceed_function = gsm_func;

      _argv.push_back(prop.function); //argv[0] = prop.name;


      //new_fd = dup(1);
      prop.arguments = reduce(prop.arguments);
      int chr;
      for (chr = 0; prop.arguments[chr] != '\0'; chr++)
      {
        if (prop.arguments[chr] == '\n' || prop.arguments[chr] == '\r')
          { prop.arguments[chr] = ' '; cout << "/!\\ Strange ASCII char found !!" << endl; }
      }

      std::istringstream iss( prop.arguments);
      //cout << "Managing arguments : [" << prop.arguments << "]" << endl;
      string token;
      int nextStr = 0;
      //_argc = 1;
      while (getline(iss, token, ' '))
      {
         token = reduce(token);
         //cout << "[ " << prop.name << " ] : " << "Managing token [" << token << "]." << endl;
         if (token == "<")
            nextStr = 1;
         else if (token == ">")
            nextStr = 2;
         else if (token != "")
         {
            if (nextStr == 1)
            {
               #if VERBOSE_OTHER
               cout << "[ " << prop.name << " ] : " << "stdIn = " << stdIn << "." << endl;
               #endif
               token.copy(stdIn, token.size());
               stdIn[token.size()] = '\0';
               //inStrm.open(stdIn);
            }
            else if (nextStr == 2)
            {
               #if VERBOSE_OTHER
               cout << "[ " << prop.name << " ] : " << "stdOut = " << stdOut << "." << endl;
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
   cout << "[ " << prop.name << " ] : " << "Executing Before." << endl;
   #endif
   //rt_task_set_priority(NULL, priority+1);
   msg.time = dataLogs->logStart();
   msg.isExecuted = 0;
   if(MoCoIsAlive && (rt_buffer_write(&bf , &msg , sizeof(monitoringMsg) , 1e6) < 0))
   {
      //MoCoIsAlive = 0;
      RT_BUFFER_INFO infos;
      rt_buffer_inquire(&bf, &infos);
      cerr << prop.name << " : failed to write BEFORE monitoring message to buffer." << "(Moco : " << MoCoIsAlive << ")" << endl
          << infos.availmem << " / " << infos.totalmem << " available on buffer " << infos.name << " " << infos.owaiters << " waiting too." << endl;
   }
   return 0;
}

void MacroTask::proceed()
{
      #if VERBOSE_OTHER
      cout << "[ " << prop.name << " ] : "<< "Executing Proceed." << endl;
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
        cerr << "["<< prop.name << " ("<< getpid() << ")] - Error during proceed ! [" << ret << "]. Function was : ";
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

      // if (std::string path(prop.function) != "/null/")  {
      //cout << prop.name << " : " << chain << endl;

      //system(&chain[0u]);
//    }
//    else cout << prop.name <<"Oups, no valid path found !" << endl;
}

int MacroTask::after()
{
   #if VERBOSE_OTHER
   cout << "[ " << prop.name << " ] : "<< "Executing After." << endl;
   #endif
   msg.time = dataLogs->logExec();
   msg.isExecuted = 1;
   if(MoCoIsAlive && (rt_buffer_write(&bf , &msg , sizeof(monitoringMsg) , 1e6) < 0))
   {
      //MoCoIsAlive = 0;
      RT_BUFFER_INFO infos;
      rt_buffer_inquire(&bf, &infos);
      cerr << prop.name << " : failed to write AFTER monitoring message to buffer." << "(Moco : " << MoCoIsAlive << ")" << endl
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
      //cout << "Task" << prop.name << " working." << endl;
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
  //cout << "Task BE " << prop.name << " not executed." << endl;
  return 0;
}

int MacroTask::after_besteff()
{
    dataLogs->logExec();
    #if VERBOSE_OTHER
    cout << "End Task : " << prop.name << endl;
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

void TaskProcess::setAffinity (int _aff, int mode)
{ // mode 0 : replace | mode 1 : add | mode -1 : remove
   cpu_set_t mask;
   if (mode == 0) { CPU_ZERO(&mask); CPU_SET(_aff, &mask); }
   else if (mode > 1) CPU_SET(_aff, &mask);
   else if (mode < -1) CPU_CLR(_aff, &mask);

   RT_TASK_INFO curtaskinfo;
   rt_task_inquire(&_task, &curtaskinfo);

   if (int ret = rt_task_set_affinity(&_task, &mask))
   {
      cerr << "Error while setting (" << mode << ") affinity for task "
      << curtaskinfo.name << " to CPU " << _aff << ": " <<  ret << "."<< endl;
   }
   #if VERBOSE_ASK
   if (mode == 0) cout << "Switched affinity for task " << curtaskinfo.name << " = CPU " << _aff << endl;
   else if (mode == 1) cout << "Added affinity for task " << curtaskinfo.name << " +CPU " << _aff << endl;
   else if (mode == -1) cout << "Removed affinity for task " << curtaskinfo.name << " -CPU " << _aff << endl;
   #endif
}

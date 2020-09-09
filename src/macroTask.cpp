#include "macroTask.h"
//#include <utmpx.h>    // Pour fonction getcpu()


bool TaskProcess::MoCoIsAlive = FALSE;

TaskProcess::TaskProcess(rtTaskInfosStruct _taskInfo)
{
   printTaskInfo(&_taskInfo);

   cout << "[ "<< _taskInfo.fP.name << " ] - "<< "Setting Real-time parameters..." << endl;
   setRTtask(_taskInfo.rtP, _taskInfo.fP.name);

   printInquireInfo(&_task);
   parseParameters(_taskInfo.fP.args);
}

void TaskProcess::setRTtask(rtPStruct _rtInfos, char* _name)
{
   //system("find /proc/xenomai");
   XENO_INIT(_name);

   int ret = 0;
   ERROR_MNG(rt_task_shadow(&_task, _name, _rtInfos.priority, 0));
   //ret = rt_task_shadow(&_task, _name, _rtInfos.priority, 0);

   rt_print_flush_buffers();
   rt_printf("[ %s ] - shadowed : %d (%s).\n", _name, ret, getErrorName(ret)); //cout << "["<< _name << "]"<< " shadowed." << endl;

   if (_rtInfos.priority != 0 || _rtInfos.schedPolicy == SCHED_OTHER)
   { // if priority = 0, SCHED_OTHER !!
      RT_TASK_INFO curtaskinfo;
      rt_task_inquire(0, &curtaskinfo);
      struct sched_param_ex param;
      param.sched_priority = _rtInfos.priority;

      if (_rtInfos.schedPolicy == SCHED_RM) _rtInfos.schedPolicy = SCHED_FIFO;
      ERROR_MNG(sched_setscheduler_ex(curtaskinfo.pid, _rtInfos.schedPolicy, &param));
      rt_printf("[ %s ] - Scheduling policy %s (%d) updated.\n", _name, getSchedPolicyName(_rtInfos.schedPolicy), _rtInfos.schedPolicy); //cout << "["<< _name << "]"<< "Managing Scheduling policy " << getSchedPolicyName(_rtInfos.schedPolicy) << endl;

      if (_rtInfos.schedPolicy == SCHED_RR)
      {
         ERROR_MNG(rt_task_slice(&_task, RR_SLICE_TIME));
         rt_printf("[ %s ] - Round-Robin slice %d ns updated.\n", _name, RR_SLICE_TIME); //cout << "["<< _name << "]"<< "Managing Scheduling policy " << getSchedPolicyName(_rtInfos.schedPolicy) << endl;
      }
   }

   ERROR_MNG(rt_task_set_priority(&_task, _rtInfos.priority));
   rt_printf("[ %s ] - Task Priority %llu updated.\n", _name, _rtInfos.priority); //cout << "["<< _name << "]"<< "Managing Scheduling policy " << getSchedPolicyName(_rtInfos.schedPolicy) << endl;

   setAffinity(_rtInfos.affinity, 0);

   //Periodicity
   ERROR_MNG(rt_task_set_periodic(&_task, TM_NOW, _rtInfos.periodicity));
   rt_printf("[ %s ] - Task Period %d updated.\n", _name, _rtInfos.periodicity); //cout << "["<< _name << "]"<< "Managing Scheduling policy " << getSchedPolicyName(_rtInfos.schedPolicy) << endl;
   rt_print_flush_buffers();

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

//ERROR_MNG( rt_task_suspend(NULL) );

}

void TaskProcess::setAffinity (int _aff, int mode)
{ // mode 0 : replace | mode 1 : add | mode -1 : remove
   cpu_set_t mask;
   if (mode == 0) { CPU_ZERO(&mask); CPU_SET(_aff, &mask); }
   else if (mode > 1) CPU_SET(_aff, &mask);
   else if (mode < -1) CPU_CLR(_aff, &mask);

   RT_TASK_INFO curtaskinfo;
   rt_task_inquire(&_task, &curtaskinfo);

   ERROR_MNG(rt_task_set_affinity(&_task, &mask));
   //#if VERBOSE_ASK
   if (mode == 0)       rt_printf("[ %s ] - Changed CPU affinity : CPU #%d.\n", curtaskinfo.name, _aff);
   else if (mode == 1)  rt_printf("[ %s ] - Added CPU affinity : +CPU #%d.\n", curtaskinfo.name, _aff);
   else if (mode == -1) rt_printf("[ %s ] - Removed CPU affinity : -CPU #%d.\n", curtaskinfo.name, _aff);
   //#endif
}

void TaskProcess::parseParameters(string _arguments)
{
   _stdIn[0] = '\0'; _stdOut[0] = '\0';
   _arguments = reduce(_arguments);

   std::istringstream iss( _arguments);
   //cout << "Managing arguments : [" << _arguments << "]" << endl;
   string token;
   int nextStr = 0;
   //_argc = 1;
   while (getline(iss, token, ' '))
   {
      token = reduce(token);
      //cout << "[ " << _name << " ] : " << "Managing token [" << token << "]." << endl;
      if (token == "<")
      nextStr = 1;
      else if (token == ">")
      nextStr = 2;
      else if (token != "")
      {
         if (nextStr == 1)
         {
            token.copy(_stdIn, token.size());
            _stdIn[token.size()] = '\0';
            #if VERBOSE_OTHER
            RT_TASK_INFO infos;
            rt_task_inquire(&_task, &infos);
            rt_printf("[ %s ] - stdIn = %s.\n", infos.name, _stdIn);
            #endif
            //inStrm.open(_stdIn);
         }
         else if (nextStr == 2)
         {
            token.copy(_stdOut, token.size());
            _stdOut[token.size()] = '\0';
            #if VERBOSE_OTHER
            RT_TASK_INFO infos;
            rt_task_inquire(&_task, &infos);
            rt_printf("[ %s ] - stdOut = %s.\n", infos.name, _stdOut);
            #endif
         }
         else
         {
            char *arg = new char[token.size() +1];  // +1
            copy(token.begin(), token.end(), arg);
            arg[token.size()]= '\0';
            #if VERBOSE_OTHER
            //rt_printf("Token : [%s] (%d) copied to [%s] (%d).\n", token.c_str(), token.size(), arg, strlen(arg));
            //cout << "token : [" << token << "] (" << token.size() << ") copied to [" << arg << "] (" << strlen(arg) << ")." << endl;
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
   rt_printf("        Arguments : ");
   int i = 0;
   for (auto arg : _argv)
   {
      string toPrint;
      if (arg==NULL)toPrint = "null";
      else toPrint = arg;
      rt_printf("Arg #%d = %s ; ", i, toPrint.c_str());
      i++;
   }
   rt_printf("\n");
   #endif
   rt_print_flush_buffers();

}

void TaskProcess::setIO()
{
   if (_stdIn[0] != '\0')
   {
      #if VERBOSE_OTHER
      rt_printf("Changed Input to : %s .\n", _stdIn);
      rt_print_flush_buffers();
      #endif
      int fdIn = open(_stdIn, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
      dup2(fdIn, 0);
      close(fdIn);
   }
   #if VERBOSE_OTHER
   else rt_printf("Unchanged Input.\n");
   #endif


   if (_stdOut[0] != '\0')
   {
      #if VERBOSE_OTHER
      rt_printf("Changed Output to : %s .\n", _stdOut);
      rt_print_flush_buffers();
      #endif
      int fdOut = open(_stdOut, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
      dup2(fdOut, 1);
      close(fdOut);
   }
   #if VERBOSE_OTHER
   else rt_printf("Unchanged Output.\n");
   #endif

}

// ================================================== //
// ======= MACRO TASK - Task Wrapper Component ====== //

MacroTask::MacroTask(rtTaskInfosStruct _taskInfo, bool MoCo, string _name) : TaskProcess(_taskInfo)
{
   MoCoIsAlive = MoCo;
   prop = _taskInfo;
   dataLogs = new TaskDataLogger(_taskInfo, _name);

   setIO( );
   findFunction(_taskInfo.fP.func);

   msg.ID      = prop.fP.id;
   msg.time    = 0;
   #if WITH_BOOL
   msg.isExecuted = 1;
   #endif

}

RTMacroTask::RTMacroTask(rtTaskInfosStruct _taskInfo, bool MoCo, string _name) : MacroTask(_taskInfo, MoCo, _name)
{}

BEMacroTask::BEMacroTask(rtTaskInfosStruct _taskInfo, bool MoCo, string _name) : MacroTask(_taskInfo, MoCo, _name)
{}

void MacroTask::findFunction (char* _func)
{
   if      (!strcmp(_func, "basicmath_small")) proceed_function = basicmath_small;
   else if (!strcmp(_func, "basicmath_large")) proceed_function = basicmath_large;
   else if (!strcmp(_func, "bitcnts"))         proceed_function = bitcount_func;
   else if (!strcmp(_func, "qsort_small"))     proceed_function = qsort_small;
   else if (!strcmp(_func, "qsort_large"))     proceed_function = qsort_large;
   else if (!strcmp(_func, "susan_bin"))       proceed_function = susan;
   else if (!strcmp(_func, "cjpeg"))           proceed_function = cjpeg_func;
   else if (!strcmp(_func, "djpeg"))           proceed_function = djpeg_func;
   else if (!strcmp(_func, "lout"))            proceed_function = typeset_func;
   else if (!strcmp(_func, "dijkstra_small"))  proceed_function = dijkstra_small;
   else if (!strcmp(_func, "dijkstra_large"))  proceed_function = dijkstra_large;
   else if (!strcmp(_func, "patricia_bin"))    proceed_function = patricia;
   else if (!strcmp(_func, "search_large"))    proceed_function = stringsearch_small;
   else if (!strcmp(_func, "search_small"))    proceed_function = stringsearch_large;
   else if (!strcmp(_func, "bf"))              proceed_function = blowfish;
   else if (!strcmp(_func, "rijndael"))        proceed_function = rijndael;
   else if (!strcmp(_func, "sha"))             proceed_function = sha;
   else if (!strcmp(_func, "adpcm_rawcaudio")) proceed_function = rawcaudio;
   else if (!strcmp(_func, "adpcm_rawdaudio")) proceed_function = rawdaudio;
   else if (!strcmp(_func, "crc"))             proceed_function = crc;
   else if (!strcmp(_func, "fft"))             proceed_function = fft;
   else if (!strcmp(_func, "toast"))           proceed_function = gsm_func;
   else if (!strcmp(_func, "untoast"))         proceed_function = gsm_func;
   else                                   proceed_function = do_load;

   _argv.insert(_argv.begin(), _func); //argv[0] = _name;

}

uint RTMacroTask::before()
{
   #if VERBOSE_OTHER
   //rt_fprintf(stderr, "[ %s ] - Executing Before.\n", prop.fP.name);
   #endif
   msg.time = dataLogs->logStart();
   #if WITH_BOOL
   msg.isExecuted = 0;
   #endif

/* VERSION AVEC ENVOI DANS BEFORE **ET** AFTER
   ret = rt_buffer_write(&_buff , &msg , sizeof(monitoringMsg) , _uSEC(500));
   if(MoCoIsAlive && (ret != sizeof(monitoringMsg)))
   {
      //MoCoIsAlive = 0;
      RT_BUFFER_INFO infos;
      ERROR_MNG(rt_buffer_inquire(&_buff, &infos));
      fprintf(stderr, "[ %s ] - ERROR %s (%d) - failed to write BEFORE monitoring message to buffer %s. (MoCo mode : %d)\n",
                     prop.fP.name, getErrorName(ret), ret, MESSAGE_TOPIC_NAME, MoCoIsAlive);
      fprintf(stderr, "Memory Available on buffer : %lu / %lu. %d waiting too.\n", infos.availmem, infos.totalmem, infos.owaiters);
      rt_print_flush_buffers();
      return -1;
   }
*/
   return 0;
}

int MacroTask::proceed()
{
   #if VERBOSE_OTHER
   //rt_fprintf(stderr, "[ %s ] - Executing Proceed.\n", prop.fP.name);
   #endif

   int ret = proceed_function(_argv.size()-1, &_argv[0]);  // -1 : no need for last element "NULL".
   if (ret != 0)
   {
      rt_fprintf(stderr, "[ %s ] - Proceed error. (%d) function was : ", prop.fP.name, ret);
      for (auto arg : _argv)
      {
         string toPrint;
         if (arg==NULL)toPrint = "null";
         else toPrint = arg;
         rt_fprintf(stderr, " %s", toPrint.c_str());
      }
      rt_fprintf(stderr, "(%d elements)\n", _argv.size()-1);
   }
   return ret;
}

int RTMacroTask::after()
{
   #if VERBOSE_OTHER
   //rt_fprintf(stderr, "[ %s ] - Executing After.\n", prop.fP.name);
   #endif
   msg.endTime = dataLogs->logExec();
   #if WITH_BOOL
   msg.isExecuted = 1;
   #endif

   ret = rt_buffer_write(&_buff , &msg , sizeof(monitoringMsg) , _mSEC(1));
   if(MoCoIsAlive && (ret != sizeof(monitoringMsg)))
   {
      //MoCoIsAlive = 0;
      RT_BUFFER_INFO infos;
      ERROR_MNG(rt_buffer_inquire(&_buff, &infos));
      fprintf(stderr, "[ %s ] - ERROR %s (%d) - failed to write AFTER monitoring message to buffer %s. (MoCo mode : %d)\n",
                     prop.fP.name, getErrorName(ret), ret, MESSAGE_TOPIC_NAME, MoCoIsAlive);
      fprintf(stderr, "Memory Available on buffer : %lu / %lu. %d waiting too.\n", infos.availmem, infos.totalmem, infos.owaiters);
      rt_print_flush_buffers();
   }
   return 0;
}

void RTMacroTask::executeRun()
{
   if (MoCoIsAlive)
   {
      ERROR_MNG(rt_buffer_bind(&_buff, MESSAGE_TOPIC_NAME, _mSEC(500)));
      //string mutexName = (string) MESSAGE_TOPIC_NAME + "_mtx";
      //ERROR_MNG(rt_mutex_bind(&_bufMtx, mutexName.c_str(), _mSEC(500)));
   }
   //rt_fprintf(stderr, "Running...\n");
   while (MoCoIsAlive)
   {
      //cout << "Task" << prop.name << " working." << endl;
      if (before() == 0) // Check if execution allowed
         if (proceed() == 0)  // execute task
            after();  // Inform of execution time for the mcAgent
      rt_task_wait_period(&dataLogs->overruns);
   }
}

////////////////////////////////////////////////////////////////////////////////////////


uint BEMacroTask::before()
{
   ERROR_MNG(rt_event_inquire(&_event, &_eventInfos));
   if ( _eventInfos.value & MODE_NOMINAL)
   {
      //dataLogs->logStart();
      return  MODE_NOMINAL;
   } else {
      rt_fprintf(stderr, "[%s] [%ld]- Stopped. \n", prop.fP.name, rt_timer_read());
      return 0;
   }


}

int BEMacroTask::after()
{
   //dataLogs->logExec();
   #if VERBOSE_OTHER
   //cout << "End Task : " << prop.fP.name << endl;
   #endif
   return 0;
}


void BEMacroTask::executeRun()
{
   //cout << "Running..." << endl;
   if (MoCoIsAlive)
   {
      ERROR_MNG(rt_event_bind(&_event, CHANGE_MODE_EVENT_NAME, _mSEC(500)));
   }
   int amount = prop.rtP.periodicity;
   for (int i = 0 ; i < amount ; ++i)
   {
      proceed();
      rt_task_wait_period(&dataLogs->overruns);

   }

   while (MoCoIsAlive)
   {
      if (before() & MODE_NOMINAL) // Check if execution allowed
      {
         if (proceed() == 0)  // execute task
            after();  // Inform of execution time for the mcAgent
      }
      else {
         rt_event_wait(&_event, MODE_NOMINAL, NULL , EV_PRIO, TM_INFINITE);
         rt_fprintf(stderr, "[%s] [%ld]- Started back. \n", prop.fP.name, rt_timer_read());
      }
      rt_task_wait_period(&dataLogs->overruns);
   }

}

void MacroTask::saveData(int maxNameSize, RT_TASK_INFO* cti)
{
   dataLogs->saveData(maxNameSize, cti);
   MoCoIsAlive = 0;
}



int do_load (int argc, char* argv[])
{
   RTIME ns = 0;
   if (argc == 2 && sscanf(argv[1], "%llu", &ns) == 1)
         rt_timer_spin( ns);
   else { rt_fprintf(stderr, "Error getting %s argument.(%d args)\n", argv[1], argc); return -1; }
   return 0;
}

/*
* make sure everybody is in the same session and that
* we have registry sharing.
*/
int do_xeno_init(char* _name)
{
   const char* args[] = {
      _name,
      //"--enable-registry"
      //"--registry-root=/usr/xenomai",
      "--shared-registry",
      "--session=test",
      //"--dump-config",
      NULL
   };
   const char **argv = args ;
   int argc = (sizeof args/sizeof args[0])-1 ; /* exclude NULL */

   xenomai_init(&argc,(char * const **)&argv) ;
   return 0;
}

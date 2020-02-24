#include <xenomai/init.h>

#include "macroTask.h"
//#include <utmpx.h>    // Pour fonction getcpu()
//#include <spawn.h>
//#include <sys/wait.h>

/*
* make sure everybody is in the same session and that
* we have registry sharing.
*/
int do_xeno_init(char* _name)
{
   const char* args[] = {
      "program",
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
#define XENO_INIT(_name) do_xeno_init(_name)

bool TaskProcess::MoCoIsAlive = FALSE;

TaskProcess::TaskProcess(rtTaskInfosStruct _taskInfo, bool MoCo)
{
   MoCoIsAlive = MoCo;
   printTaskInfo(&_taskInfo);

   cout << "[ "<< _taskInfo.fP.name << " ] - "<< "Setting Real-time parameters..." << endl;
   setRTtask(_taskInfo.rtP, _taskInfo.fP.name);

   printInquireInfo(&_task);

   parseParameters(_taskInfo.fP.args);
   rt_print_flush_buffers();
   setIO( );
}

void TaskProcess::setRTtask(rtPStruct _rtInfos, char* _name)
{
   //system("find /proc/xenomai");
   XENO_INIT(_name) ;

   int ret = 0;
   //ERROR_MNG(rt_task_shadow(_task, _name, _rtInfos.priority, 0));
   ret = rt_task_shadow(&_task, _name, _rtInfos.priority, 0);
   rt_print_flush_buffers();
   rt_printf("[ %s ] - shadowed : %d (%s).\n", _name, ret, getErrorName(ret)); //cout << "["<< _name << "]"<< " shadowed." << endl;

   RT_TASK_INFO curtaskinfo;
   rt_task_inquire(0, &curtaskinfo);
   struct sched_param_ex param;
   param.sched_priority = _rtInfos.priority;
   /* sched_param
   param.sched_flags= 0;
   param.sched_runtime = taskInfo.periodicity;;
   param.sched_deadline = taskInfo.periodicity;
   param.sched_period = taskInfo->periodicity;
   param.size=sizeof(sched_attr);
   old... */
   const char* schedName = getSchedPolicyName(_rtInfos.schedPolicy);
   rt_printf("[ %s ] - Managing Scheduling Policy %s (%d).\n", _name, schedName, _rtInfos.schedPolicy); //cout << "["<< _name << "]"<< "Managing Scheduling policy " << getSchedPolicyName(_rtInfos.schedPolicy) << endl;

   if (_rtInfos.schedPolicy == SCHED_RM) _rtInfos.schedPolicy = SCHED_FIFO;
   if( (ret = sched_setscheduler_ex(curtaskinfo.pid, _rtInfos.schedPolicy, &param)) != 0)
   {
      cerr << "[ " << _name << " ] "<< curtaskinfo.pid << " : error setting scheduling policy " << _rtInfos.schedPolicy << ", Error #" << strerror(ret) << endl;
      exit(ret);
   }

   if (_rtInfos.schedPolicy == SCHED_RR)
   { //#if defined SCHED_POLICY  &&  SCHED_POLICY == SCHED_RR
      if ((ret = rt_task_slice(&_task, RR_SLICE_TIME)))
      { cerr << "[ " << _name << " ] " << "Slice Error : " << ret << " ." << endl; exit(-4); }
   } //#endif

   //Periodicity
   if ((ret = rt_task_set_periodic(&_task, TM_NOW, _rtInfos.periodicity)))
   {  cerr << "[ " << _name << " ] " << "Set_Period Error : " << ret << " ." << endl; exit(-2); }

   setAffinity(_rtInfos.affinity, 0);

   if ((ret = rt_task_set_priority(&_task, _rtInfos.priority)))
   { cerr << "[ " << _name << " ] " << "Set_Priority Error : " << ret << " ." << endl; exit(-5); }
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

   if (int ret = rt_task_set_affinity(&_task, &mask))
   {
      cerr << "Error while setting (" << mode << ") affinity for task "
      << curtaskinfo.name << " to CPU " << _aff << ": " <<  ret << "."<< endl;
   }
   #if VERBOSE_ASK
   if (mode == 0) rt_printf("[ %s ] - Switched affinity : CPU %d.\n", curtaskinfo.name, _aff);
   else if (mode == 1) rt_printf("[ %s ] - Added affinity : +CPU %d.\n", curtaskinfo.name, _aff);
   else if (mode == -1) rt_printf("[ %s ] - Removed affinity : -CPU %d.\n", curtaskinfo.name, _aff);
   #endif
}

void TaskProcess::parseParameters(string _arguments)
{
   _stdIn[0] = '\0'; _stdOut[0] = '\0';

   /*int chr;
   for (chr = 0; _arguments[chr] != '\0'; chr++)
   {
      if ((_arguments[chr] <= 0x20 || _arguments[chr] >= 0x7A) && _arguments[chr] != ' ')
      {
         _arguments[chr] = ' ';
         rt_fprintf(stderr, "/!\\ Strange ASCII char found !! (%c)\n", _arguments[chr]);
      }
   }*/
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
            rt_printf("Token : [%s] (%d) copied to [%s] (%d).\n", token.c_str(), token.size(), arg, strlen(arg));
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
}

void TaskProcess::setIO( )
{
    /*
          if (stdIn[0] != '\0')
              std::cin.rdbuf(inStrm.rdbuf()); //redirect std::cin to stdIn!

          if (stdOut[0] != '\0')
              std::cout.rdbuf(outStrm.rdbuf()); //redirect std::cout to stdOut!
    */
   if (_stdIn[0] != '\0')
   {
      #if VERBOSE_OTHER
      rt_printf("Changed Input to : %s .\n", _stdIn);
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
      #endif
      int fdOut = open(_stdOut, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
      dup2(fdOut, 1);
      close(fdOut);
   }
   #if VERBOSE_OTHER
   else rt_printf("Unchanged Output.\n");
   #endif
}

MacroTask::MacroTask(rtTaskInfosStruct _taskInfo, bool MoCo) : TaskProcess(_taskInfo, MoCo)
{
   prop = _taskInfo;
   dataLogs = new TaskDataLogger(&prop, _stdOut);
   findFunction(_taskInfo.fP.func);

   msg.task    = &_task;
   msg.ID      = prop.fP.id;
   msg.time    = 0;
   msg.isExecuted = 0;

}

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
   else if (!strcmp(_func, "sha_bin"))         proceed_function = sha;
   else if (!strcmp(_func, "adpcm_rawcaudio")) proceed_function = rawcaudio;
   else if (!strcmp(_func, "adpcm_rawdaudio")) proceed_function = rawdaudio;
   else if (!strcmp(_func, "crc"))             proceed_function = crc;
   else if (!strcmp(_func, "fft"))             proceed_function = fft;
   else if (!strcmp(_func, "toast"))           proceed_function = gsm_func;
   else if (!strcmp(_func, "untoast"))         proceed_function = gsm_func;

   _argv.insert(_argv.begin(), _func); //argv[0] = _name;

}

int MacroTask::before()
{
   //   rt_mutex_acquire(&mutex, TM_INFINITE);
   #if VERBOSE_OTHER
   cout << "[ " << prop.fP.name << " ] : " << "Executing Before." << endl;
   #endif
   //rt_task_set_priority(NULL, priority+1);
   msg.time = dataLogs->logStart();
   msg.isExecuted = 0;
   if(MoCoIsAlive && (rt_buffer_write(&_buff , &msg , sizeof(monitoringMsg) , 1e6) < 0))
   {
      //MoCoIsAlive = 0;
      RT_BUFFER_INFO infos;
      rt_buffer_inquire(&_buff, &infos);
      cerr << prop.fP.name << " : failed to write BEFORE monitoring message to buffer." << "(Moco : " << MoCoIsAlive << ")" << endl
      << infos.availmem << " / " << infos.totalmem << " available on buffer " << infos.name << " " << infos.owaiters << " waiting too." << endl;
   }
   return 0;
}

void MacroTask::proceed()
{
   #if VERBOSE_OTHER
   cout << "[ " << prop.fP.name << " ] : "<< "Executing Proceed." << endl;
   #endif

   int ret = proceed_function(_argv.size()-1, &_argv[0]);  // -1 : no need for last element "NULL".
   if (ret != 0)
   {
      cerr << "["<< prop.fP.name << " ("<< getpid() << ")] - Error during proceed ! [" << ret << "]. Function was : ";
      int i = 0;
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
}

int MacroTask::after()
{
   #if VERBOSE_OTHER
   cout << "[ " << prop.fP.name << " ] : "<< "Executing After." << endl;
   #endif
   msg.time = dataLogs->logExec();
   msg.isExecuted = 1;
   if(MoCoIsAlive && (rt_buffer_write(&_buff , &msg , sizeof(monitoringMsg) , 1e6) < 0))
   {
      //MoCoIsAlive = 0;
      RT_BUFFER_INFO infos;
      rt_buffer_inquire(&_buff, &infos);
      cerr << prop.fP.name << " : failed to write AFTER monitoring message to buffer." << "(Moco : " << MoCoIsAlive << ")" << endl
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
      ERROR_MNG(rt_buffer_bind (&_buff , MESSAGE_TOPIC_NAME, 1*1e9));// 1s timeout
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
   dataLogs->logStart();
   ERROR_MNG(rt_event_inquire(&_event, &_eventInfos));
   return _eventInfos.value;
}

int MacroTask::after_besteff()
{
   dataLogs->logExec();
   #if VERBOSE_OTHER
   cout << "End Task : " << prop.fP.name << endl;
   #endif
   //   rt_mutex_release(&mutex);
   // rt_task_set_priority(NULL, priority);
   return 0;
}


void MacroTask::executeRun_besteffort()
{
   //cout << "Running..." << endl;
   ERROR_MNG(rt_event_bind(&_event, CHANGE_MODE_EVENT_NAME, 10*1e9)); // 5 seconds timeout
   unsigned int flag;
   while (1)
   {
      if (before_besteff() == MODE_NOMINAL) // Check if execution allowed
         proceed();  // execute task
      else rt_event_wait(&_event, sizeof(flag), &flag , EV_PRIO, TM_INFINITE);

         after_besteff();  // Inform of execution time for the mcAgent

      rt_task_wait_period(&dataLogs->overruns);

   }
}

void MacroTask::finishProcess(void* _task)
{
   MacroTask* task = (MacroTask*) _task;
   task->dataLogs->saveData(task->prop.fP.name, 32);

}

const char* getSchedPolicyName(int schedPol)
{
   switch (schedPol)
   {
      case SCHED_FIFO      :
         return "FIFO\0";
      break;
      case SCHED_RR        :
         return "Round-Robin\0";
      break;
      case SCHED_WEAK      :
         return "Weak\0";
      break;
      case SCHED_COBALT    :
         return "Cobalt\0";
      break;
      case SCHED_SPORADIC  :
         return "Sporadic\0";
      break;
      case SCHED_TP        :
         return "TimePartitioning\0";
      break;
      case SCHED_QUOTA     :
         return "QUOTA\0";
      break;
      case SCHED_EDF       :
         return "Earliest Deadline First\0";
      break;
      case SCHED_RM        :
         return "Rate-Monotonic\0";
      break;
      default : return "Undefined Policy\0";
      break;
   }
};

const char* getErrorName(int err)
{
   switch (err)
   {
      case 0   :
         return "No Error.\0";
      case -EINTR  :
         return "EINTR\0";
         break;
      case -EWOULDBLOCK  :
         return "EWOULDBLOCK\0";
         break;
      case -ETIMEDOUT :
         return "ETIMEDOUT\0";
         break;
      case -EPERM  :
         return "EPERM\0";
         break;
      case -EEXIST :
         return "EEXIST\0";
         break;
      case -ENOMEM :
         return "ENOMEM\0";
         break;
      case -EINVAL :
         return "EINVAL\0";
         break;
      case -EDEADLK   :
         return "EDEADLK\0";
         break;
      case -ESRCH  :
         return "ESRCH\0";
         break;
      case -EBUSY  :
         return "EBUSY\0";
         break;
      default: return "Undefined Error Code.\0";
   }
}

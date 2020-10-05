#include "tools.h"
#include <sys/sysinfo.h>


std::string trim(const std::string& str,
                 const std::string& whitespace)
{
    const auto strBegin = str.find_first_not_of(whitespace);
    if (strBegin == std::string::npos)
        return ""; // no content

    const auto strEnd = str.find_last_not_of(whitespace);
    const auto strRange = strEnd - strBegin + 1;

    return str.substr(strBegin, strRange);
}

std::string reduce(const std::string& str,
                   const std::string& fill,
                   const std::string& whitespace)
{
   string tmp = str;
   for (int chr = 0; tmp[chr] != '\0'; chr++)
   {
      if ((tmp[chr] <= 0x20 || tmp[chr] >= 0x7A) && tmp[chr] != ' ')
      {
         tmp[chr] = ' ';
         fprintf(stderr, "/!\\ Strange ASCII char found !! (%c)\n", tmp[chr]);
      }
   }
    // trim first
    auto result = trim(tmp, whitespace);

    // replace sub ranges
    auto beginSpace = result.find_first_of(whitespace);
    while (beginSpace != std::string::npos)
    {
        const auto endSpace = result.find_first_not_of(whitespace, beginSpace);
        const auto range = endSpace - beginSpace;

        result.replace(beginSpace, range, fill);

        const auto newStart = beginSpace + fill.length();
        beginSpace = result.find_first_of(whitespace, newStart);
    }

    return result;
}

void printInquireInfo(RT_TASK* task)
{
   #if VERBOSE_INFO
   RT_TASK_INFO curtaskinfo;
   int ret = 0;
   if ((ret = rt_task_inquire(task, &curtaskinfo)))
   {  cout << "\n Inquire Error (" << ret;
      if (ret == -EINVAL) cerr << ") - Invalid Task Descriptor or invalid Prio." << endl;
      if (ret == -EPERM) cerr << ") - Task is NULL, and service called from invalid context." << endl;
   } else {
      rt_printf("[ %s ] - (PID : %d) - Priority = %d.\n", curtaskinfo.name, curtaskinfo.pid, curtaskinfo.prio);
   }
   rt_print_flush_buffers();
   #endif
}

void printTaskInfo(rtTaskInfosStruct* task)
{
   #if VERBOSE_OTHER
   std::stringstream ss;
   ss << "   ___ ["       << task->fP.name << "] - parameter summary : "
      << "\n" << "     |- ID :"       << task->fP.id << " (isHRT : " << task->fP.isHRT << ")"
      << "\n" << "     |- func : "     << task->fP.func << " | Args : " << task->fP.args
      << "\n" << "     |- Period: "   << task->rtP.periodicity
              << " | on core : " << task->rtP.affinity
              << " | at priority : " << task->rtP.priority << " | Policy: "   << getSchedPolicyName(task->rtP.schedPolicy) << " (" << task->rtP.schedPolicy << ")"
      << "\n" << "     |- WCET = " << task->fP.wcet << " | Precedent task ID : " << task->fP.prec
   << endl;
   cout << ss.rdbuf();
   #endif
}

void print_affinity(pid_t _pid)
{
   #if VERBOSE_ASK
   long nproc = get_nprocs();
   int pid = _pid;
   if (!_pid)
   {
      RT_TASK_INFO curtaskinfo;
      rt_task_inquire(NULL, &curtaskinfo);
      pid = curtaskinfo.pid;
   }
   cpu_set_t mask;

   if (sched_getaffinity(_pid, sizeof(cpu_set_t), &mask) == -1) {
      perror("sched_getaffinity");
      assert(false);
  } else {
    long i;
    std::stringstream ss;
    ss << "Affinity of thread " << pid << " = ";
    for (i = 0; i < nproc; i++)
        ss << CPU_ISSET(i, &mask);
    ss << endl;
    cout << ss.rdbuf();
    /* using printf
    printf("sched_getaffinity = ");
    for (i = 0; i < nproc; i++) {
        printf("%d ", CPU_ISSET(i, &mask));
    }
    printf("\n");
    */
  }
#endif
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
   switch (abs(err))
   {
      case 0   :
         return "No Error.\0";
      case EINTR  :
         return "EINTR\0";
         break;
      case EWOULDBLOCK  : // = EAGAIN
         return "EWOULDBLOCK or EAGAIN\0";
         break;
      case ETIMEDOUT :
         return "ETIMEDOUT\0";
         break;
      case EPERM  :
         return "EPERM\0";
         break;
      case EEXIST :
         return "EEXIST\0";
         break;
      case ENOMEM :
         return "ENOMEM\0";
         break;
      case EINVAL :
         return "EINVAL\0";
         break;
      case EDEADLK   :
         return "EDEADLK\0";
         break;
      case ESRCH  :
         return "ESRCH\0";
         break;
      case EBUSY  :
         return "EBUSY\0";
         break;
      case EFAULT  :
         return "EFAULT\0";
         break;
      case EIDRM  :
         return "EIDRM\0";
         break;
      case EMFILE  :
         return "EMFILE\0";
         break;
      default: return "Undefined Error Code.\0";
   }
}

#include "tools.h"

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
         rt_fprintf(stderr, "/!\\ Strange ASCII char found !! (%c)\n", tmp[chr]);
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


/*
int main(void)
{
    const std::string foo = "    too much\t   \tspace\t\t\t  ";
    const std::string bar = "one\ntwo";

    std::cout << "[" << trim(foo) << "]" << std::endl;
    std::cout << "[" << reduce(foo) << "]" << std::endl;
    std::cout << "[" << reduce(foo, "-") << "]" << std::endl;

    std::cout << "[" << trim(bar) << "]" << std::endl;
}

Result:

[too much               space]
[too much space]
[too-much-space]
[one
two]
*/

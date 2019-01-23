#include "tools.h"

#include "mcAgent.h"
#include "macroTask.h"


void RunmcAgentMain(void *arg);
void runTasks(std::vector<rtTaskInfos>);
std::vector<rtTaskInfos> readTasksList(string);
void printTasksInfos ( std::vector<rtTaskInfos> );

int main(int argc, char* argv[])
{
  int return_code = 0;

  // get input file, either indicated by user as argument or default location
  string input_file;
  if (argc > 1) input_file = argv[1];
  else input_file = "./input.txt";
  std::vector<rtTaskInfos> tasksInfos = readTasksList(input_file);

  printTasksInfos(tasksInfos);
  std::cout << "I'm okay till here !" << std::endl;
  runTasks(tasksInfos);
  /* To create a task :
   * Arguments : &task,
   *             name,
   *             stack size (def = 0)
   *             priority
   *             mode (FPU, Start suspended, ...)
   */
//  RT_TASK mcAgent;
//  int rep = rt_task_create(&mcAgent, "Hello", 0, 50, 0);

  /* To start a task
   * Arguments : &task,
   *             task main
   *             function Arguments
   */
//   rt_task_start(&mcAgent, RunTask, &tasksInfos);




  return return_code;
}

void RunmcAgentMain(void *arg)
{
  MCAgent* mcAgent = new MCAgent();
}



void RunTask(void* arg)
{
  std::cout << "Running !!!";
  std::vector<rtTaskInfos> rtTI = *(std::vector<rtTaskInfos>*) arg;
  printTasksInfos (rtTI);

  RT_TASK_INFO curtaskinfo;
  rt_task_inquire(NULL, &curtaskinfo);
  std::cout << "I am task : " << curtaskinfo.name << " of priority " << curtaskinfo.prio << std::endl;
}

std::vector<rtTaskInfos> readTasksList(string input_file)
{

  system("clear");
  std::cout << "Initialising machine...\n";
  std::ifstream myFile(input_file);
  if (!myFile.is_open())
  {
      exit(EXIT_FAILURE);
  }

  std::vector<rtTaskInfos> myTasksInfos;

  string str;
  std::getline(myFile, str); // skip the first line
  while (std::getline(myFile, str))
  {
      rtTaskInfos taskInfo;
      std::istringstream iss(str);
      string token;
      //std::cout << "Managing line : " << str << std::endl;
      if (!(iss >> taskInfo.name
                >> taskInfo.path
                >> taskInfo.isHardRealTime
                >> taskInfo.periodicity
                >> taskInfo.deadline
                >> taskInfo.affinity) )
      { std::cout << "FAIL !" << std::endl; break; } // error

      myTasksInfos.push_back(taskInfo);
  }

  return myTasksInfos;

}

void printTasksInfos ( std::vector<rtTaskInfos> _myTasksInfos)
{
  for (auto &taskInfo : _myTasksInfos)
  {
    std::cout << "name: " << taskInfo.name
              << "| path: " << taskInfo.path
              << "| is RT ? " << taskInfo.isHardRealTime
              << "| Period: " << taskInfo.periodicity
              << "| Deadline: " << taskInfo.deadline
              << "| affinity: " << taskInfo.affinity << std::endl;
  }
}

void printTaskInfo(rtTaskInfos task)
{
  std::cout << "name: " << task.name
            << "| path: " << task.path
            << "| is RT ? " << task.isHardRealTime
            << "| Period: " << task.periodicity
            << "| Deadline: " << task.deadline
            << "| affinity: " << task.affinity << std::endl;
}

void runTasks(std::vector<rtTaskInfos> _myTasksInfos)
{
  for (auto taskInfo = _myTasksInfos.begin(); taskInfo != _myTasksInfos.end(); ++taskInfo)
  {
      RT_TASK* task = new RT_TASK;

      rt_task_create(task, taskInfo->name, 0, 50, 0);
      std::cout << "Task " << taskInfo->name << " created." << std::endl;
      int rep = rt_task_start(task, RunTask, &taskInfo);
      std::cout << "Task " << taskInfo->name << " started." << std::endl;
  }
}

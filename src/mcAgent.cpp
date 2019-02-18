#include "mcAgent.h"
#include "tools.h"

#define MODE_OVERLOADED     1
#define MODE_NOMINAL        0

MCAgent::MCAgent(void *arg)
{
  printInquireInfo();
  print_affinity(0);

  systemRTInfo* sInfos = (systemRTInfo*) arg;
  //TasksInformations = rtTI;

  triggerCount = 0;
  runtimeMode = MODE_NOMINAL;
  displaySystemInfo(sInfos);

  initMoCoAgent(sInfos);


    while(false)
    {

      for (auto _taskChain = allTaskChain.begin(); _taskChain != allTaskChain.end(); ++_taskChain)
      {
        if ( !_taskChain->checkTaskE2E() )
        {
          setMode(MODE_OVERLOADED);
          _taskChain->isAtRisk = 1;
        }
        if (_taskChain->isAtRisk && _taskChain->checkIfEnded())
        {
          setMode(MODE_NOMINAL);
          _taskChain->resetChain();
        }
      }
    }
}

void MCAgent::initMoCoAgent(systemRTInfo* sInfos)
{
  setAllDeadlines(*sInfos->e2eDD);
  setAllTasks(*sInfos->rtTIs);
}


/***********************
* Création des différentes chaines de tâche avec e2e deadline associée
* @params : std::vector<end2endDeadlineStruct> _tcDeadlineStructs
*           liste d' ID de chaines de tâche avec e2e deadline associées
* @returns : /
***********************/
void MCAgent::setAllDeadlines(std::vector<end2endDeadlineStruct> _tcDeadlineStructs)
{
  for (auto& tcDeadlineStruct : _tcDeadlineStructs)
  {
    taskChain tc(tcDeadlineStruct);
    allTaskChain.push_back(tc);
  }
}

/***********************
* Ajout des tâches critiques aux Tasks Chains
* Les tâches sont parcourues et ajoutées à leur groupe.
* @params : std::vector<rtTaskInfosStruct> _TasksInfos
*           liste des tâches d'entrée.
* @returns : /
***********************/
void MCAgent::setAllTasks(std::vector<rtTaskInfosStruct> _TasksInfos)
{
  for (auto& _taskInfo : _TasksInfos)
  {
    bool idFound = 0;   // Opti. pour éviter de continuer à boucler si on a trouvé la chaine
    for (auto& _taskChain : allTaskChain)
    {
      if ( !_taskInfo.isHardRealTime )
      {
        bestEffortTasks.push_back(_taskInfo.task);
        idFound = 1;
      }
      else if ( _taskInfo.isHardRealTime == _taskChain.id )
      {
        taskMonitoringStruct tms(_taskInfo);
        _taskChain.taskList.push_back(tms);
        idFound = 1;
      }

      if (idFound) return;
    }
  }
}

/***********************
* Check all taskChains respects of deadline constraints
* Return bit mask of unrespectful chains
* @params : /
* @returns : int tasksID
***********************/
int MCAgent::checkTaskChains()
{
  int tasksID = 0;
  for (auto _taskChain = allTaskChain.begin(); _taskChain != allTaskChain.end(); ++_taskChain)
  {
    if ( !_taskChain->checkTaskE2E() )
    {
      setMode(MODE_OVERLOADED);
      _taskChain->isAtRisk = 1;
      tasksID = _taskChain->id;
    }
  }
  return tasksID;
}

/***********************
* Passage du système en mode :
* MODE_NOMINAL : Toutes les tâches passent.
* MODE_OVERLOADED : Les tâches BE ne sont pas lancées.
* Envoi un signal Condition de MODE
* Parcours toutes les tâches best effort pour Suspend/Resume
***********************/
void MCAgent::setMode(int mode)
{
  runtimeMode += 2*mode - 1;  // NOMINAL : -1 ; OVERLOADED : +1
  if (runtimeMode == MODE_OVERLOADED)
  { // Pause Best Effort Tasks;
    for (auto& bestEffortTask : bestEffortTasks)
    {   // Publier message pour dire à stopper ??;


    }
  }
  else // runtimeMode NOMINAL
  {
    for (auto& bestEffortTask : bestEffortTasks)
    {  // relancer Best Effort Tasks;


    }
  }
}

/***********************
* Fonction de débug pour afficher
* les informations de toutes les tâches reçues.
* @params : [ systemRTInfo sInfos ]
* @returns : cout
***********************/
void MCAgent::displaySystemInfo(systemRTInfo* sInfos)
{
  #if VERBOSE_INFO
  cout << "INPUT Informations : ";
  for (auto &taskdd : *sInfos->e2eDD)
  { // Print chain params
      cout << "Chain ID : " << taskdd.taskChainID
          << "| Deadline : " << taskdd.deadline << endl;
  }
  for (auto &taskParam : *sInfos->rtTIs)
  { // Print task Params
      cout << "Name: "    << taskParam.name
          << "| path: "   << taskParam.path
          << "| is RT ? " << taskParam.isHardRealTime
          << "| Period: " << taskParam.periodicity
          << "| Deadline: " << taskParam.deadline
          << "| affinity: " << taskParam.affinity << endl;
  }
  #endif
}

taskChain::taskChain(end2endDeadlineStruct _tcDeadline)
{
  id = _tcDeadline.taskChainID;
  end2endDeadline = _tcDeadline.deadline;
}

/***********************
* Fonction limite respect de deadline
* @params : [ systemRTInfo sInfos ]
* @returns : 1 if OK ; 0 if RISK
***********************/
int taskChain::checkTaskE2E()
{
  return (getExecutionTime() + Wmax + offset + getRemWCET() <= end2endDeadline);
}

int taskChain::checkIfEnded()
{
  bool ended = 1; // 1 stands for TRUE
  for (auto& task : taskList)
  { // only one FALSE is enough
    ended = ended && task.isExecuted;
  }
  if (ended) resetChain();
  return ended;
}

void taskChain::resetChain()
{
  for (auto& task : taskList) task.isExecuted = 0;
}

double taskChain::getExecutionTime()
{
    return (currentEndTime - startTime);
}

/***********************
* Compute Remaining WCET of Task Chain
* Considered as : Sum of WCET of remaining tasks.
* @params : /
* @returns : double RemWCET
***********************/
double taskChain::getRemWCET()
{
  double RemWCET = 0;
  for (auto& task : taskList)
  {
    if (!task.isExecuted)
      RemWCET += task.rwcet;
  }
  return RemWCET;
}


/***********************
* Recoit la liste des tasksInfos lues depuis l'INPUT.txt
* Le converti avec uniquement les informations nécessaires pour le MoCoAgent
* @params : vec<rtTaskInfosStruct> rtTasks
* @returns : [ vector<taskMonitoringStruct> taskList ]
***********************/
taskMonitoringStruct::taskMonitoringStruct(rtTaskInfosStruct rtTaskInfos)
{
  task = rtTaskInfos.task;
  deadline = rtTaskInfos.deadline;
  rwcet = rtTaskInfos.deadline;

  isExecuted = 0;
  startTime = 0, endTime = 0;
}

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
  displayInformations(sInfos->rtTIs);

  runtimeMode = MODE_NOMINAL;
  displaySystemInfo(sInfos);

  initMoCoAgent(sInfos);
    while(false)
    {
      if (runtimeMode == MODE_OVERLOADED)
      {

      } else
      if(runtimeMode == MODE_NOMINAL)
      {

      }
      checkTasks();
    }
}

void MCAgent::initMoCoAgent(systemRTInfo* sInfos)
{
  setAllDeadlines(*sInfos->e2eDD);
  setAllTasks(*sInfos->rtTIs);
<<<<<<< HEAD
=======
}


int MCAgent::checkTasks()
{
  for (auto _taskChain = allTaskChain.begin(); _taskChain != allTaskChain.end(); ++_taskChain)
  {
    if (_taskChain->checkTaskE2E() )
    {
      setMode(MODE_OVERLOADED);
    }
  }
  return 0;
>>>>>>> 04353966915521c4d2abb16bb5899412d9438841
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
<<<<<<< HEAD
    bool idFound = 0;   // Opti. pour éviter de continuer à boucler si on a trouvé la chaine
    for (auto& _taskChain : allTaskChain)
    {
      if ( !_taskChain.isHardRealTime )
      {
        bestEffortTasks.push_back(_taskChain.task);
        idFound = 1;
      }
      else if ( _taskInfo.isHardRealTime == _taskChain.id )
      {
=======
    bool idFound = 0;
    for (auto& _taskChain : allTaskChain)
    {
      if (_taskInfo.isHardRealTime == _taskChain.id)
      {
>>>>>>> 04353966915521c4d2abb16bb5899412d9438841
        taskMonitoringStruct tms(_taskInfo);
        _taskChain.taskChainList.push_back(tms);
        idFound = 1;
      }

      if (idFound) return;
    }
  }
}


int MCAgent::checkTasks()
{
  for (auto _taskChain = allTaskChain.begin(); _taskChain != allTaskChain.end(); ++_taskChain)
  {
    if (_taskChain->checkTaskE2E() )
    {
      setMode(MODE_OVERLOADED);
    }
  }
  return 0;
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
  runtimeMode = mode;
  for (auto bestEffortTask : bestEffortTasks)
  {
      if (runtimeMode == MODE_OVERLOADED)
      {
        // Pause Best Effort Tasks;
        // Publier message pour dire à stopper ??
      }
      else // runtimeMode NOMINAL
      {
        // relancer Best Effort Tasks;
      }
  }

}

/***********************
* Fonction de débug pour afficher
* les informations de toutes les tâches reçues.
<<<<<<< HEAD
* @params : [ systemRTInfo sInfos ]
* @returns : cout
***********************/
void MCAgent::displaySystemInfo(systemRTInfo* sInfos)
=======
* @params : [ vect<rtTaskInfosStruct>* TasksInformations ]
* @returns : cout
***********************/
void MCAgent::displayInformations(std::vector<rtTaskInfosStruct>* TasksInformations)
>>>>>>> 04353966915521c4d2abb16bb5899412d9438841
{
  #if VERBOSE_INFO
  cout << "INPUT Informations : ";
  for (auto &taskdd : *sInfos->e2edd)
  {
      cout << "Chain ID : " << taskdd.taskChainID
          << "| Deadline : " << taskdd.deadline << endl;
  }
  for (auto &taskParam : *sInfos->rtTIs)
  {
      cout << "Name: "    << taskParam.name
          << "| path: "   << taskParam.path
          << "| is RT ? " << taskParam.isHardRealTime
          << "| Period: " << taskParam.periodicity
          << "| Deadline: " << taskParam.deadline
          << "| affinity: " << taskParam.affinity << endl;
  }
  #endif

<<<<<<< HEAD
=======
taskChain::taskChain(end2endDeadlineStruct _tcDeadline)
{
  id = _tcDeadline.taskChainID;
  end2endDeadline = _tcDeadline.deadline;
  slackTime = 0;
>>>>>>> 04353966915521c4d2abb16bb5899412d9438841
}

/***********************
* Recoit la liste des tasksInfos lues depuis l'INPUT.txt
* Le converti avec uniquement les informations nécessaires pour le MoCoAgent
* @params : vec<rtTaskInfosStruct> rtTasks
* @returns : [ vector<taskMonitoringStruct> taskChainList ]
***********************/
taskMonitoringStruct::taskMonitoringStruct(rtTaskInfosStruct rtTaskInfos)
{
  task = rtTaskInfos.task;
  deadline = rtTaskInfos.deadline;
  rwcet = rtTaskInfos.deadline;

  isExecuted = 0;
  execTime = 0;
}

taskChain::taskChain(end2endDeadlineStruct _tcDeadline)
{
  id = _tcDeadline.taskChainID;
  end2endDeadline = _tcDeadline.deadline;
  slackTime = 0;
}

int taskChain::checkTaskE2E()
{

  return 0;


  return 1;
}

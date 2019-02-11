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
  displayInformations();

  runtimeMode = MODE_NOMINAL;

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

void initMoCoAgent(std::vector<rtTaskInfosStruct>* sInfos)
{
  setAllDeadlines(sInfos->e2eDD);
  setAllTasks(sInfos->rtTIs);
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
* Création des différentes chaines de tâche avec e2e deadline associée
* @params : std::vector<end2endDeadlineStruct> _tcDeadlineStructs
*           liste d' ID de chaines de tâche avec e2e deadline associées
* @returns : /
***********************/
void MCAgent::setAllTaskDeadlines(std::vector<end2endDeadlineStruct> _tcDeadlineStructs)
{
  for (auto& tcDeadlineStruct : _tcDeadlineStructs)
  {
    taskChain tc = new taskChain(tcDeadlineStruct);
    allTaskChain.push_back(tc);
  }
}


void MCAgent::setAllTasks(std::vector<rtTaskInfosStruct> _TasksInfos)
{
  for (auto& _taskInfo : _TasksInfos)
  {
    bool idFound = FALSE;
    for (auto& _taskChain : allTaskChain)
    {
      if (_taskInfo->isHardRealTime == _taskChain->id)
      {
        taskMonitoringStruct tms = new taskMonitoringStruct(_taskInfo);
        _taskChain.taskChainList.push_back(tms);
        idFound == TRUE;
      }
      if (idFound)  return;
    }
  }
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

}

/***********************
* Fonction de débug pour afficher
* les informations de toutes les tâches reçues.
* @params : [ vect<rtTaskInfosStruct>* TasksInformations ]
* @returns : cout.
***********************/
void MCAgent::displayInformations()
{
  #if VERBOSE_INFO
  for (auto &taskInfo : *TasksInformations)
  {
      cout << "Name: " << taskInfo.name
          << "| path: " << taskInfo.path
          << "| is RT ? " << taskInfo.isHardRealTime
          << "| Period: " << taskInfo.periodicity
          << "| Deadline: " << taskInfo.deadline
          << "| affinity: " << taskInfo.affinity << endl;
  }
  #endif
}

taskChain::taskChain(end2endDeadlineStruct _tcDeadline)
{
  id = _tcDeadline.id;
  end2endDeadline = _tcDeadline.deadline;
  slackTime = 0;
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

  isExecuted = FALSE;
  execTime = 0;
}

int taskChain::checkTaskE2E()
{

  return 0;


  return 1;
}

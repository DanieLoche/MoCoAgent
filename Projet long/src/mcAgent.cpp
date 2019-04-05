#include "mcAgent.h"

void messageReceiver(void* arg)
{
  MCAgent* mocoAgent = (MCAgent*) arg;

  while(TRUE)
  {
  //  void* _msg;
    monitoringMsg msg;
  //  cout<<"buffer read"<<endl;
    rt_buffer_read(&mocoAgent->bf, &msg, sizeof(monitoringMsg), TM_INFINITE);
  //  cout<<"done buffer read"<<endl;
    mocoAgent->updateTaskInfo(msg);


  }
}

MCAgent::MCAgent(void *arg)
{
  printInquireInfo();
  print_affinity(0);

  systemRTInfo* sInfos = (systemRTInfo*) arg;
  //TasksInformations = rtTI;

  runtimeMode = MODE_NOMINAL;
  displaySystemInfo(sInfos);

  initMoCoAgent(sInfos);
  initCommunications();

  RT_TASK mcAgentReceiver;
  rt_task_create(&mcAgentReceiver, "MoCoAgentReceiver", 0, 99, 0);
  rt_task_start(&mcAgentReceiver, messageReceiver, this);
  while(TRUE)
  {
      for (auto _taskChain = allTaskChain.begin(); _taskChain != allTaskChain.end(); ++_taskChain)
      {
        if ( !_taskChain->checkTaskE2E() )
        {
          setMode(MODE_OVERLOADED);
          _taskChain->isAtRisk = TRUE;
        }
        if (_taskChain->isAtRisk && _taskChain->checkIfEnded())
        {
          setMode(MODE_NOMINAL);
          _taskChain->resetChain();
        }
      }
      rt_task_yield();
   }
}

void MCAgent::initMoCoAgent(systemRTInfo* sInfos)
{
  setAllDeadlines(sInfos->e2eDD);
  setAllTasks(sInfos->rtTIs);
  displayChains();
}

void MCAgent::initCommunications()
{
  rt_buffer_create(&bf, "/monitoringTopic", 10*sizeof(monitoringMsg), B_FIFO);
  // u_int flagMask = 0;
  rt_event_create(&mode_change_flag, "/modeChangeTopic", 0, EV_PRIO);
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
    taskChain* tc = new taskChain(tcDeadlineStruct);
    allTaskChain.push_back(*tc);
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
        taskMonitoringStruct* tms = new taskMonitoringStruct(_taskInfo);
        _taskChain.taskList.push_back(*tms);
        idFound = 1;
      }
      if (idFound) break;

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
    if ( _taskChain->checkTaskE2E() )
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
  /*
  #if VERBOSE_DEBUG
    cout << "MoCoAgent Triggered to mode " << mode << "!" << endl;
  #endif

  runtimeMode += 2*mode - 1;  // NOMINAL : -1 ; OVERLOADED : +1
  if (runtimeMode == MODE_OVERLOADED)
  { // Pause Best Effort Tasks;
    rt_event_signal(&mode_change_flag, 1);
    for (auto& bestEffortTask : bestEffortTasks)
    {   // Publier message pour dire à stopper
      rt_task_suspend(bestEffortTask);
    }
  }
  else // runtimeMode NOMINAL
  {
    rt_event_signal(&mode_change_flag, 0);
    for (auto& bestEffortTask : bestEffortTasks)
    {  // relancer Best Effort Tasks;
      rt_task_resume(bestEffortTask);
    }
  }
  */
}

// METHODE A OPTIMISER SUR LE PARCOURS
// DES TACHE EN ORDONNANCANT LES TAKS LISTS.
void MCAgent::updateTaskInfo(monitoringMsg msg)
{
/*  RT_TASK_INFO curtaskinfo;
  rt_task_inquire((RT_TASK*) msg.task, &curtaskinfo);
  cout << "I am task : " << curtaskinfo.name<<"ID :"<< msg.ID<< endl;
*/
  std::vector <taskMonitoringStruct> tk_List;

  for (auto& _taskChain : allTaskChain)
  {
      for (auto& task : _taskChain.taskList)
      {
        if( task.id == msg.ID)
        {
          if (!msg.startTime)
            task.startTime = msg.startTime;
          else if (msg.isExecuted)
          {
            task.endTime = msg.endTime;
             task.isExecuted = TRUE;
          }
          return;
        }
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
  for (auto &taskdd : sInfos->e2eDD)
  { // Print chain params
      cout << "Chain ID : " << taskdd.taskChainID
          << "| Deadline : " << taskdd.deadline << endl;
  }
  for (auto &taskParam : sInfos->rtTIs)
  { // Print task Params
      cout << "Name: "    << taskParam.name
          << "| path: "   << taskParam.path_task
          << "| is RT ? " << taskParam.isHardRealTime
          << "| Period: " << taskParam.periodicity
          << "| Deadline: " << taskParam.deadline
          << "| affinity: " << taskParam.affinity
          << "| ID :"<< taskParam.ID << endl;
  }
  #endif
}

void MCAgent::displayChains()
{
  for (auto& chain : allTaskChain)
  {
    #if VERBOSE_ASK
      cout << "Chain #" << chain.id << " with deadline = " << chain.end2endDeadline << endl;
    #endif
    chain.displayTasks();
  }
}

//////////////////////////////////////////
/////////// TASK CHAIN CLASS /////////////
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
  return (getExecutionTime() + Wmax + t_RT + getRemWCET() <= end2endDeadline);
}

int taskChain::checkIfEnded()
{
  bool ended = TRUE;
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
  isAtRisk = FALSE;
  startTime = 0;
}

double taskChain::getExecutionTime()
{
    return (currentEndTime - startTime);
}

/***********************
* Compute Remaining WCET of Task Chain
* Considered as : Sum of WCET of remaining tasks.
* @params : /
* @returns : RTIME RemWCET
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

void taskChain::displayTasks()
{
  #if VERBOSE_ASK
    cout << "Task list : " << endl;
    for (auto& task : taskList)
    {
      cout << "    - Task ID #" << task.id << "proper deadline = " << task.deadline << endl;
    }
  #endif
}

/////////////////////////////////////////////////
//////////// TASK MONITORING STRUCTURE //////////

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
  id = rtTaskInfos.ID;

  isExecuted = FALSE;
  startTime = 0, endTime = 0;
}

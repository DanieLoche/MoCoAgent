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

MCAgent::MCAgent(systemRTInfo* sInfos)
{
  //printInquireInfo();
  //print_affinity(0);
  //TasksInformations = rtTI;

  runtimeMode = MODE_NOMINAL;
  displaySystemInfo(sInfos);

  initCommunications();
  initMoCoAgent(sInfos);

  #if VERBOSE_INFO
    cout << "MoCoAgent Ready." << endl;
  #endif

  while(TRUE)
  {
      for (auto _taskChain = allTaskChain.begin(); _taskChain != allTaskChain.end(); ++_taskChain)
      {
        if ( _taskChain->checkTaskE2E() )
        {
          _taskChain->isAtRisk = TRUE;
          _taskChain->cptAnticipatedMisses++;
          setMode(MODE_OVERLOADED);
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

  RT_TASK mcAgentReceiver;
  rt_task_create(&mcAgentReceiver, "MoCoAgentReceiver", 0, 99, 0);
  rt_task_start(&mcAgentReceiver, messageReceiver, this);
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
  #if VERBOSE_INFO
  cout << "[MoCoAgent] : setting deadlines." << endl;
  #endif
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
  #if VERBOSE_INFO
  cout << "[MoCoAgent] : setting tasks." << endl;
  #endif
  for (auto& _taskInfo : _TasksInfos)
  {
    if ( !_taskInfo.isHardRealTime )
    {
      bestEffortTasks.push_back(_taskInfo.task);
      return;
    }
    for (auto& _taskChain : allTaskChain)
    {
      if ( _taskInfo.isHardRealTime == _taskChain.id )
      {
        taskMonitoringStruct* tms = new taskMonitoringStruct(_taskInfo);
        _taskChain.taskList.push_back(*tms);
        return;
      }
    }
  }
}

/***********************
* Check all taskChains respects of deadline constraints
* Return bit mask of unrespectful chains
* @params : /
* @returns : int tasksID
***********************/
/*
int MCAgent::checkTaskChains()
{
  int tasksID = 0;
  for (auto _taskChain = allTaskChain.begin(); _taskChain != allTaskChain.end(); ++_taskChain)
  {
    if ( _taskChain->checkTaskE2E() )
    {
      setMode(MODE_OVERLOADED);
      _taskChain->isAtRisk = TRUE;
      _taskChain->cptAnticipatedMisses++;
      tasksID = _taskChain->id;
    }
  }
  return tasksID;
}
*/

/***********************
* Passage du système en mode :
* MODE_NOMINAL : Toutes les tâches passent.
* MODE_OVERLOADED : Les tâches BE ne sont pas lancées.
* Envoi un signal Condition de MODE
* Parcours toutes les tâches best effort pour Suspend/Resume
***********************/
void MCAgent::setMode(int mode)
{

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

}

// METHODE A OPTIMISER SUR LE PARCOURS
// DES TACHE EN ORDONNANCANT LES TAKS LISTS.
void MCAgent::updateTaskInfo(monitoringMsg msg)
{
  /*  RT_TASK_INFO curtaskinfo;
    rt_task_inquire((RT_TASK*) msg.task, &curtaskinfo);
    cout << "I am task : " << curtaskinfo.name<<"ID :"<< msg.ID<< endl;
  */
  for (auto& _taskChain : allTaskChain)
  {
      for (auto& task : _taskChain.taskList)
      {
        if( task.id == msg.ID)
        {
          if (msg.isExecuted)
          {
            //task.endTime = msg.time;
            task.setState(TRUE);
            _taskChain.currentEndTime = std::max(_taskChain.currentEndTime, msg.time);
            if (_taskChain.checkIfEnded() && _taskChain.isAtRisk) setMode(MODE_NOMINAL);
          }
          else if (_taskChain.startTime == 0)
          {
            _taskChain.startTime = msg.time;
            _taskChain.logger->logStart(msg.time);
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
  cout << "INPUT Informations : " << endl;
  for (auto &taskdd : sInfos->e2eDD)
  { // Print chain params
      cout << "Chain ID : " << taskdd.taskChainID
          << "| Deadline : " << taskdd.deadline /1.0e6 << endl;
  }
  for (auto &taskParam : sInfos->rtTIs)
  { // Print task Params
      cout << "Name: "    << taskParam.name
          << "| path: "   << taskParam.path_task
          << "| is RT ? " << taskParam.isHardRealTime
          << "| WCET: " << taskParam.wcet /1.0e6
          << "| Deadline: " << taskParam.deadline /1.0e6
          << "| affinity: " << taskParam.affinity
          << "| ID :"<< taskParam.id << endl;
  }
  #endif
}

void MCAgent::displayChains()
{
  for (auto& chain : allTaskChain)
  {
    #if VERBOSE_ASK
      cout << "Chain #" << chain.id << " with deadline = " << chain.end2endDeadline/1.0e6 << endl;
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
  logger = new DataLogger(&_tcDeadline);
  //cptOutOfDeadline = 0;
  cptAnticipatedMisses = 0;
  startTime = 0;
  currentEndTime = 0;
  //chainExecutionTime = {0};
}

/***********************
* Fonction limite respect de deadline
* @params : [ systemRTInfo sInfos ]
* @returns : 1 if OK ; 0 if RISK
***********************/
bool taskChain::checkTaskE2E()
{
  bool miss = (getExecutionTime() + Wmax + t_RT + getRemWCET() > end2endDeadline);
  //if (miss) cptAnticipatedMisses++;
  return (miss);
}

bool taskChain::checkIfEnded()
{
  bool ended = TRUE;
  for (auto& task : taskList)
  { // only one FALSE is enough
    ended = ended && task.getState();
  }
  if (ended)
  {
    resetChain();
    logger->logExec(currentEndTime);
    //RTIME execTime = getExecutionTime();
    //if (execTime > end2endDeadline) cptOutOfDeadline++;
    //chainExecutionTime[cptExecutions++] = execTime;
  }
  return ended;
}

void taskChain::resetChain()
{
  for (auto& task : taskList)
  {
    task.setState(0);
    //task.endTime = 0;
  }
  if (isAtRisk)
  {
     isAtRisk = FALSE;
  }
  startTime = 0;
  currentEndTime = 0;
}

RTIME taskChain::getExecutionTime()
{
    return (rt_timer_read() - startTime);
}

/***********************
* Compute Remaining WCET of Task Chain
* Considered as : Sum of WCET of remaining tasks.
* @params : /
* @returns : RTIME RemWCET
***********************/
RTIME taskChain::getRemWCET()
{
  double RemWCET = 0;
  for (auto& task : taskList)
  {
    if (!task.getState())
      RemWCET += task.rwcet;
  }
  return RemWCET;
}

void taskChain::displayTasks()
{
  #if VERBOSE_INFO
    cout << "Task list : " << endl;
    for (auto& task : taskList)
    {
      cout << "    - Task ID #" << task.id << " - deadline = " << task.deadline/1.0e6 << endl;
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
  tache = rtTaskInfos.task;
  deadline = rtTaskInfos.deadline;
  rwcet = rtTaskInfos.wcet;
  id = rtTaskInfos.id;

  setState(FALSE);
  //endTime = 0;

  rt_mutex_create(mtx_taskStatus, NULL);
}

bool taskMonitoringStruct::getState()
{
  rt_mutex_acquire(mtx_taskStatus, TM_INFINITE);
  bool state = isExecuted;
  rt_mutex_release(mtx_taskStatus);
  return state;
}

#include "mcAgent.h"


void messageReceiver(void* arg)
{
  MCAgent* mocoAgent = (MCAgent*) arg;
  monitoringMsg msg;
  while(TRUE)
  {
  //  cout<<"buffer read"<<endl;
    rt_buffer_read(&mocoAgent->bf, &msg, sizeof(monitoringMsg), TM_INFINITE);
  //  cout<<"done buffer read"<<endl;
    mocoAgent->updateTaskInfo(msg);
  // rt_task_yield();

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

}

void MCAgent::execute()
{
   while(TRUE)
   {
     if (runtimeMode)
     {
         for (auto& _taskChain : allTaskChain)
         {
            if ( _taskChain->checkTaskE2E() && !_taskChain->isAtRisk)
            {
             _taskChain->isAtRisk = TRUE;
             _taskChain->logger->cptAnticipatedMisses++;
             setMode(MODE_OVERLOADED);
            }
         }
     }
     if (*triggerSave)
     {
         saveData();
         pause();
     }

    rt_task_yield();
    }
}


void MCAgent::initMoCoAgent(systemRTInfo* sInfos)
{
   outputFileName = sInfos->outputFileName;
   triggerSave = sInfos->triggerSave;
   setAllDeadlines(sInfos->e2eDD);
   setAllTasks(sInfos->rtTIs);
   displayChains();
   sleep(1);
   RT_TASK mcAgentReceiver;
   rt_task_create(&mcAgentReceiver, "MoCoAgentReceiver", 0, 99, 0);
   rt_task_start(&mcAgentReceiver, messageReceiver, this);
}

void MCAgent::initCommunications()
{
   rt_buffer_create(&bf, "/monitoringTopic", 20*sizeof(monitoringMsg), B_FIFO);
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
      if (tcDeadlineStruct.taskChainID)
      {
         taskChain* tc = new taskChain(tcDeadlineStruct);
         allTaskChain.push_back(tc);
      }
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
    cout << " Adding task " << _taskInfo.name << " (" << _taskInfo.isHardRealTime << ")." << endl;
    printInquireInfo(_taskInfo.task);
    if ( !_taskInfo.isHardRealTime )
    {
      cout << "Adding to BE list." << endl;
      bestEffortTasks.push_back(_taskInfo.task);
      break;
    }
    else for (auto& _taskChain : allTaskChain)
    {
      if ( _taskInfo.isHardRealTime == _taskChain->chainID )
      {
         cout << "Adding to HRT chain " << _taskChain->chainID << "." << endl;
        taskMonitoringStruct* tms = new taskMonitoringStruct(&_taskInfo);
        _taskChain->taskList.push_back(*tms);
        break;
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
/* USELESS ?
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
   //cout << "[MONITORING & CONTROL AGENT] Change mode to " << ((mode>0)?"OVERLOADED":"NOMINAL") << ". " << endl;
   if (!mode)  runtimeMode = mode;
   else if (bestEffortTasks.size() != 0)
   {
      if (mode >= MODE_OVERLOADED && runtimeMode <= MODE_NOMINAL)
      { // Pause Best Effort Tasks sur front montant changement de mode.
         rt_event_signal(&mode_change_flag, 1);
         for (auto& bestEffortTask : bestEffortTasks)
         {   // Publier message pour dire à stopper
            if (rt_task_suspend(bestEffortTask))
            {
               #if VERBOSE_DEBUG // ==1?"OVERLOADED":"NOMINAL"
               cerr << "Failed to stop task ";
               #endif
               //printInquireInfo(bestEffortTask);
            }
         }
         #if VERBOSE_DEBUG // ==1?"OVERLOADED":"NOMINAL"
         cout << "Stopped BE tasks." << endl;
         #endif
      }
      else if (mode <= MODE_NOMINAL && runtimeMode >= MODE_OVERLOADED)
      { // runtimeMode NOMINAL
         rt_event_signal(&mode_change_flag, 0);
         for (auto& bestEffortTask : bestEffortTasks)
         {  // relancer Best Effort Tasks;
            rt_task_resume(bestEffortTask);
         }
         #if VERBOSE_DEBUG // ==1?"OVERLOADED":"NOMINAL"
         cout << "Re-started BE tasks." << endl;
         #endif
      }
      runtimeMode = mode;
      #if VERBOSE_ASK // ==1?"OVERLOADED":"NOMINAL"
      cout << "MoCoAgent Triggered to mode " << ((mode>0)?"OVERLOADED":"NOMINAL") << "!" << endl;
      #endif
   }
}

// METHODE OPTIMISABLE SUR LE PARCOURS
// DES TACHE EN TRIANT LES TAKS LISTS.
void MCAgent::updateTaskInfo(monitoringMsg msg)
{
   // RT_TASK_INFO curtaskinfo;
   // rt_task_inquire((RT_TASK*) msg.task, &curtaskinfo);
   // cout << "Update from task : " << curtaskinfo.name<<" ("<< msg.ID << ") - " << msg.isExecuted << " T=" << msg.time/1e6 << endl;
   // printInquireInfo((RT_TASK*) msg.task);
   for (auto& _taskChain : allTaskChain)
   {
      for (auto& _task : _taskChain->taskList)
      {
               //cout << "Logged chain end at : " << _taskChain->currentEndTime/1.0e6 << endl;
            //cout << "Logged chain start at : " << msg.time/1.0e6 << endl;
         if( _task.id == msg.ID )
         {
             if (_task.precedencyID == 0 && _taskChain->startTime == 0)
             { // First task of the chain
                _taskChain->startTime = msg.time;
                _taskChain->logger->logStart(msg.time);
                _task.setState(TRUE);
             }
             else if (msg.isExecuted && _taskChain->checkPrecedency(_task.precedencyID))
             { // Next task of the chain
               _taskChain->currentEndTime = std::max(_taskChain->currentEndTime, msg.time);
               _task.setState(TRUE);
               if (_taskChain->checkIfEnded())
               {
                  _taskChain->logger->logExec(_taskChain->currentEndTime);
                  if (_taskChain->isAtRisk) setMode(MODE_NOMINAL);
                  _taskChain->resetChain();
               }
             }
             return;
         }
      }
   }
}

void MCAgent::saveData()
{
   cout << "SAVING MoCoAgent datas" << endl;
   std::ofstream outputFileResume;
   string file = outputFileName + "_Resume.txt";
   outputFileResume.open (file, std::ios::app);    // TO APPEND :  //,ios_base::app);
   RT_TASK_INFO cti;
   rt_task_inquire(0, &cti);
   //"MoCoLogs_" +
   outputFileResume << "\n Monitoring and Control Agent Stats : \n"
                    << "Primary Mode execution time - " << cti.stat.xtime/1.0e6 << " ms."
                    << " Timeouts : " << cti.stat.timeout << "\n"
                    << "   Mode Switches - " << cti.stat.msw << "\n"
                    << "Context Switches - " << cti.stat.csw << "\n"
                    << "Cobalt Sys calls - " << cti.stat.xsc
                    << endl;
   outputFileResume.close();

    for (auto _taskChain : allTaskChain)
    {
        _taskChain->logger->saveData(outputFileName, 0);
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
   cout << "\n I RECEIVED TRIGGER BOOLEAN " << *(sInfos->triggerSave) << endl;
   sleep(1);
  #if VERBOSE_INFO
  cout << "INPUT Informations : " << endl;
  for (auto &taskdd : sInfos->e2eDD)
  { // Print chain params
      cout << "|- Chain: " << taskdd.name       << "\n"
           << " ‾|- ID      : " << taskdd.taskChainID       << "\n"
           << "   |- Deadline: " << taskdd.deadline /1.0e6   << "\n"
           << "   |- Path    : " << taskdd.Path             // << "\n"
           << endl;
  }
  for (auto &taskParam : sInfos->rtTIs)
  { // Print task Params
      cout << "  |- Task    : "  << taskParam.name            << "\n"
           << "   ‾|- ID      : " << taskParam.id               << "\n"
           << "     |- Period  : " << taskParam.periodicity /1.0e6  << "\n"
           << "     |- WCET    : " << taskParam.wcet /1.0e6      << "\n"
           << "     |- affinity: " << taskParam.affinity         << "\n"
           << "     |- RealTime: " << taskParam.isHardRealTime   << "\n"
           << "     |- path    : " << taskParam.path_task      //  << "\n"
           << endl;
  }
  #endif
}

void MCAgent::displayChains()
{
  #if VERBOSE_ASK
  cout << "Displaying MoCoAgent Database : " << "\n";
  for (auto& chain : allTaskChain)
  { // Print chain params
      cout << "|- Chain #" << chain.chainID << " - " << chain.name
           << "  |- Deadline: " << chain.end2endDeadline /1.0e6 << " ms."
           << endl;
      chain.displayTasks();
  }
  #endif
}

//////////////////////////////////////////
/////////// TASK CHAIN CLASS /////////////
taskChain::taskChain(end2endDeadlineStruct _tcDeadline)
{
   strcpy(name, _tcDeadline.name);
   chainID = _tcDeadline.taskChainID;
   end2endDeadline = _tcDeadline.deadline;
   logger = new ChainDataLogger(&_tcDeadline);
   startTime = 0;
   currentEndTime = 0;
}

/***********************
* Fonction limite respect de deadline
* @params : [ systemRTInfo sInfos ]
* @returns : 1 if OK ; 0 if RISK
***********************/
bool taskChain::checkTaskE2E()
{
   bool miss = 0;
   if (startTime != 0)
   {
      RTIME execTime = getExecutionTime();
      RTIME remTime = getRemWCET();
      miss = ( execTime + Wmax + t_RT + remTime > end2endDeadline );
      //cout << "Exec Time : " << execTime/1e6 << " | Rem Time : " << remTime/1e6 << " | Deadline : " << end2endDeadline/1e6 << endl;
   }
   //if (miss) cptAnticipatedMisses++;
   return (miss);
}

bool taskChain::checkPrecedency(int _id)
{ // Attention ! Précédence entre tâche géré que avec une tâche antérieure..!
   bool isOkay = FALSE;
   for (auto& task : taskList)
   { // Plus simple à coder..!
      isOkay = isOkay || ((task.id == _id) && task.getState());
   }
   return isOkay;
}

bool taskChain::checkIfEnded()
{
   bool ended = TRUE;
   for (auto& task : taskList)
   { // only one FALSE is enough
    ended = ended & task.getState();
   }
   return ended;
}

void taskChain::resetChain()
{
   for (auto& task : taskList)
   {
      task.setState(FALSE);
      //task.endTime = 0;
   }
   if (isAtRisk) isAtRisk = FALSE;
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
  RTIME RemWCET = 0;
  for (auto& task : taskList)
  {
    bool state = task.getState();
    //cout << state << "-";
    if (!state)
      RemWCET += task.rwcet;
  }
  return RemWCET;
}

void taskChain::displayTasks()
{
  #if VERBOSE_INFO
  cout << "Task list : " << endl;
  for (auto &_task : taskList)
  { // Print task Params
      RT_TASK_INFO xenoInfos;
      rt_task_inquire(_task.xenoTask, &xenoInfos);
      cout << "  |- Task    : "  << xenoInfos.name            << "\n"
           << "   ‾‾|- ID      : " << _task.id               << "\n"
           << "     |- Deadline: " << _task.deadline /1.0e6  << "\n"
           << "     |- WCET    : " << _task.rwcet /1.0e6     << "\n"
           << "     |- Priority: " << xenoInfos.prio      //   << "\n"
           << endl;
  }
  #endif
}

/////////////////////////////////////////////////
//////////// TASK MONITORING STRUCTURE //////////
/////////////////////////////////////////////////
/***********************
* Recoit la liste des tasksInfos lues depuis l'INPUT.txt
* Le converti avec uniquement les informations nécessaires pour le MoCoAgent
* @params : vec<rtTaskInfosStruct> rtTasks
* @returns : [ vector<taskMonitoringStruct> taskList ]
***********************/
taskMonitoringStruct::taskMonitoringStruct(rtTaskInfosStruct* rtTaskInfos)
{
   xenoTask = rtTaskInfos->task;
   deadline = rtTaskInfos->periodicity;
   rwcet = rtTaskInfos->wcet;
   id = rtTaskInfos->id;
   precedencyID = rtTaskInfos->precedency;

   setState(FALSE);
   //endTime = 0;

   rt_mutex_create(&mtx_taskStatus,
                  strncat(rtTaskInfos->name,
                           "taskStatusMTX",
                           sizeof(rtTaskInfos->name)+sizeof("taskStatusMTX")));

}

void taskMonitoringStruct::setState(bool state)
{
  rt_mutex_acquire(&mtx_taskStatus, TM_INFINITE);
  isExecuted = state;
  rt_mutex_release(&mtx_taskStatus);
}

bool taskMonitoringStruct::getState()
{
  rt_mutex_acquire(&mtx_taskStatus, TM_INFINITE);
  bool state = isExecuted;
  rt_mutex_release(&mtx_taskStatus);
  return state;
}

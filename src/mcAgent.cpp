#include "macroTask.h"


void messageReceiver(void* arg)
{
   MCAgent* mocoAgent = (MCAgent*) arg;
   monitoringMsg msg;
   while(TRUE)
   {
      //  cout<<"buffer read"<<endl;
      rt_buffer_read(&mocoAgent->_buff, &msg, sizeof(monitoringMsg), TM_INFINITE);
      //  cout<<"done buffer read"<<endl;
      mocoAgent->updateTaskInfo(msg);
      // rt_task_wait_period(&mocoAgent->overruns);
      // rt_task_yield();

   }
}

MCAgent::MCAgent(rtTaskInfosStruct _taskInfo,
                  std::vector<end2endDeadlineStruct> e2eDD,
                  std::vector<rtTaskInfosStruct> tasksSet) : TaskProcess (_taskInfo, TRUE)
{
    displaySystemInfo(e2eDD, tasksSet);
    runtimeMode = MODE_NOMINAL;
    initCommunications();

    setAllDeadlines(e2eDD);
    setAllTasks(tasksSet);
    displayChains();
    //sleep(1);

    #if VERBOSE_INFO
    rt_printf("[ MoCoAgent ] : READY.\n");

    #endif
}

void MCAgent::initCommunications()
{
   rt_buffer_create(&_buff, MESSAGE_TOPIC_NAME, 20*sizeof(monitoringMsg), B_FIFO);
   rt_event_create(&_event, CHANGE_MODE_EVENT_NAME, 0, EV_PRIO);
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
   rt_printf("[ MoCoAgent ] - setting deadlines.\n");
   #endif
   for (auto& tcDeadlineStruct : _tcDeadlineStructs)
   {
      if (tcDeadlineStruct.taskChainID)
      {
         taskChain* tc = new taskChain(tcDeadlineStruct, _stdOut);
         allTaskChain.push_back(*tc);
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
   rt_printf("[ MoCoAgent ] - setting tasks.\n");
   #endif
   for (auto& _taskInfo : _TasksInfos)
   {
      rt_printf(" Adding task %s (%d).\n", _taskInfo.fP.name, _taskInfo.fP.isHRT);
      if ( !_taskInfo.fP.isHRT )
      {
         rt_printf("Adding %s to BE list.\n", _taskInfo.fP.name);
         RT_TASK _t;
         ERROR_MNG(rt_task_bind(&_t, _taskInfo.fP.name, TM_NONBLOCK));
         printInquireInfo(&_t);
         bestEffortTasks.push_back(_t);
         break;
      }
      else for (auto& _taskChain : allTaskChain)
      {
         if ( _taskInfo.fP.isHRT == _taskChain.chainID )
         {
            rt_printf("Adding %s to HRT chain %d.\n", _taskInfo.fP.name, _taskChain.chainID);
            taskMonitoringStruct* tms = new taskMonitoringStruct(_taskInfo);
            _taskChain.taskList.push_back(*tms);
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
         rt_event_signal(&_event, MODE_OVERLOADED);
         for (auto& bestEffortTask : bestEffortTasks)
         {   // Publier message pour dire à stopper
            if (rt_task_suspend(&bestEffortTask))
            {
               #if VERBOSE_DEBUG
               cerr << "Failed to stop task "; // ==1?"OVERLOADED":"NOMINAL"
               #endif
               //printInquireInfo(bestEffortTask);
            }
         }
         #if VERBOSE_DEBUG
         cerr << "Stopped BE tasks." << endl; // ==1?"OVERLOADED":"NOMINAL"
         #endif
      }
      else if (mode <= MODE_NOMINAL && runtimeMode >= MODE_OVERLOADED)
      { // runtimeMode NOMINAL
         rt_event_signal(&_event, MODE_NOMINAL);
         for (auto& bestEffortTask : bestEffortTasks)
         {  // relancer Best Effort Tasks;
            rt_task_resume(&bestEffortTask);
         }
         #if VERBOSE_DEBUG // ==1?"OVERLOADED":"NOMINAL"
         cerr << "Re-started BE tasks." << endl;
         #endif
      }
      runtimeMode = mode;
      #if VERBOSE_ASK // ==1?"OVERLOADED":"NOMINAL"
      cerr << "MoCoAgent Triggered to mode " << ((mode>0)?"OVERLOADED":"NOMINAL") << "!" << endl;
      #endif
   }
}

const timespec noWait_time = {0,0};
void MCAgent::executeRun()
{
   int ret_msg = 0;
   while(TRUE)
   {
      ret_msg = rt_buffer_read_timed(&_buff, &msg, sizeof(monitoringMsg), &noWait_time);
      switch(ret_msg)
      {
         case -EWOULDBLOCK : // no message received
         for (auto& _taskChain : allTaskChain)
         {
            if ( _taskChain.checkTaskE2E() && !_taskChain.isAtRisk)
            {
               _taskChain.isAtRisk = TRUE;
               _taskChain.logger->cptAnticipatedMisses++;
               setMode(MODE_OVERLOADED);
            }
         }
         break;
         case -ETIMEDOUT :
         case -EINTR :
         case -EINVAL :
         case -EIDRM :
         case -EPERM :
         cerr << "[MOCOAGENT] Error on receiving message, code [" << ret_msg << "]" << endl;
         break;
         default :
         for (auto& _taskChain : allTaskChain)
         {
            for (auto& _task : _taskChain.taskList)
            {
               //cout << "Logged chain end at : " << _taskChain->currentEndTime/1.0e6 << endl;
               //cout << "Logged chain start at : " << msg.time/1.0e6 << endl;
               if( _task.id == msg.ID )
               {
                  if (_task.precedencyID == 0 && _taskChain.startTime == 0)
                  { // First task of the chain
                     _taskChain.startTime = msg.time;
                     _taskChain.logger->logStart(msg.time);
                     _task.setState(TRUE);
                  }
                  else if (msg.isExecuted && _taskChain.checkPrecedency(_task.precedencyID))
                  { // Next task of the chain
                     _taskChain.currentEndTime = msg.time;
                     _task.setState(TRUE);
                     if (_taskChain.checkIfEnded())
                     {
                        _taskChain.logger->logExec(_taskChain.currentEndTime);
                        if (_taskChain.isAtRisk) setMode(MODE_NOMINAL);
                        _taskChain.resetChain();
                     }
                  }
                  break;
               }
            }
            // Execute part //
            if ( _taskChain.checkTaskE2E() && !_taskChain.isAtRisk)
            {
               _taskChain.isAtRisk = TRUE;
               _taskChain.logger->cptAnticipatedMisses++;
               setMode(MODE_OVERLOADED);
            }
         }

      }

      rt_task_wait_period(&overruns);
   }
}

void MCAgent::executeRun_besteffort()
{
   while(TRUE)
   {
      // Update Msg part //
      rt_buffer_read(&_buff, &msg, sizeof(monitoringMsg), TM_INFINITE);
      for (auto& _taskChain : allTaskChain)
      {
         for (auto& _task : _taskChain.taskList)
         {
            //cout << "Logged chain end at : " << _taskChain->currentEndTime/1.0e6 << endl;
            //cout << "Logged chain start at : " << msg.time/1.0e6 << endl;
            if( _task.id == msg.ID )
            {
               if (_task.precedencyID == 0 && _taskChain.startTime == 0)
               { // First task of the chain
                  _taskChain.startTime = msg.time;
                  _taskChain.logger->logStart(msg.time);
                  _task.setState(TRUE);
               }
               else if (msg.isExecuted && _taskChain.checkPrecedency(_task.precedencyID))
               { // Next task of the chain
                  _taskChain.currentEndTime = msg.time;
                  _task.setState(TRUE);
                  if (_taskChain.checkIfEnded())
                  {
                     _taskChain.logger->logExec(_taskChain.currentEndTime);
                     _taskChain.resetChain();
                  }
               }
               break;
            }
         }
         // Execute part //
         if ( _taskChain.checkTaskE2E() && !_taskChain.isAtRisk)
         {
            _taskChain.isAtRisk = TRUE;
            _taskChain.logger->cptAnticipatedMisses++;
         }
      }
      rt_task_wait_period(&overruns);
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
      for (auto& _task : _taskChain.taskList)
      {
         //cout << "Logged chain end at : " << _taskChain->currentEndTime/1.0e6 << endl;
         //cout << "Logged chain start at : " << msg.time/1.0e6 << endl;
         if( _task.id == msg.ID )
         {
            if (_task.precedencyID == 0 && _taskChain.startTime == 0)
            { // First task of the chain
               _taskChain.startTime = msg.time;
               _taskChain.logger->logStart(msg.time);
               _task.setState(TRUE);
            }
            else if (msg.isExecuted && _taskChain.checkPrecedency(_task.precedencyID))
            { // Next task of the chain
               _taskChain.currentEndTime = std::max(_taskChain.currentEndTime, msg.time);
               _task.setState(TRUE);
               if (_taskChain.checkIfEnded())
               {
                  _taskChain.logger->logExec(_taskChain.currentEndTime);
                  if (_taskChain.isAtRisk) setMode(MODE_NOMINAL);
                  _taskChain.resetChain();
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
   string s = _stdOut;
   string file = s + "_Resume.txt";
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
        _taskChain.logger->saveData(_stdOut, 0);
    }

}

/***********************
* Fonction de débug pour afficher
* les informations de toutes les tâches reçues.
* @params : [ systemRTInfo sInfos ]
* @returns : cout
***********************/
void MCAgent::displaySystemInfo(std::vector<end2endDeadlineStruct> _e2eDD,
                                 std::vector<rtTaskInfosStruct> _tasksSet)
{
   #if VERBOSE_INFO
   cout << "- MoCoAgent input database -" << endl;
   cout << "------- Task  Chains -------" << endl;
   for (auto &taskdd : _e2eDD)
   { // Print chain params
      cout << "|- Chain: "      << taskdd.name       << "\n"
           << "‾|- ID      : " << taskdd.taskChainID       << "\n"
           << "  |- Deadline: " << taskdd.deadline /1.0e6   << "\n"
           << "  |- Path    : " << taskdd.Path             // << "\n"
           << endl;
   }
   cout << "-------- Tasks List --------" << endl;
   for (auto &taskParam : _tasksSet)
   { // Print task Params
      cout << "  |- Task    : "  << taskParam.fP.name            << "\n"
           << "  ‾|- ID      : " << taskParam.fP.id               << "\n"
           << "    |- Period  : " << taskParam.rtP.periodicity /1.0e6  << "\n"
           << "    |- WCET    : " << taskParam.fP.wcet /1.0e6      << "\n"
           << "    |- affinity: " << taskParam.rtP.affinity         << "\n"
           << "    |- RealTime: " << taskParam.fP.isHRT   << "\n"
           << "    |- path    : " << taskParam.fP.func      //  << "\n"
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
taskChain::taskChain(end2endDeadlineStruct _tcDeadline, string outfile)
{
   strcpy(name, _tcDeadline.name);
   chainID = _tcDeadline.taskChainID;
   end2endDeadline = _tcDeadline.deadline;
   logger = new ChainDataLogger(&_tcDeadline, outfile);
   startTime = 0;
   currentEndTime = 0;
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
      rt_task_inquire(&_task.xenoTask, &xenoInfos);
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
taskMonitoringStruct::taskMonitoringStruct(rtTaskInfosStruct _taskInfos)
{
   ERROR_MNG(rt_task_bind(&xenoTask, _taskInfos.fP.name, TM_NONBLOCK));
   deadline = _taskInfos.rtP.periodicity;
   rwcet = _taskInfos.fP.wcet;
   id = _taskInfos.fP.id;
   precedencyID = _taskInfos.fP.prec;

   setState(FALSE);
   //endTime = 0;
   char mutexName[38];
   strcpy(mutexName, _taskInfos.fP.name);
   strncat(mutexName, "_StMTX", 6);
   rt_mutex_create(&mtx_taskStatus, mutexName);

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

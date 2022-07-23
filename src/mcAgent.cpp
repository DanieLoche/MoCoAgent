#include "macroTask.h"

Agent::Agent(rtTaskInfosStruct _taskInfo,
                  std::vector<end2endDeadlineStruct> e2eDD,
                  std::vector<rtTaskInfosStruct> tasksSet) : TaskProcess (_taskInfo)
{
   MoCoIsAlive = FALSE;

   displaySystemInfo(e2eDD, tasksSet);
   runtimeMode = MODE_NOMINAL;

   setAllChains(e2eDD);
   setAllTasks(tasksSet);

   displayChains();

   #if VERBOSE_INFO
   rt_printf("[ Agent ] - READY.\n");
   #endif
}

MonitoringAgent::MonitoringAgent(rtTaskInfosStruct _taskInfo,
                  std::vector<end2endDeadlineStruct> e2eDD,
                  std::vector<rtTaskInfosStruct> tasksSet) :
                  Agent(_taskInfo, e2eDD, tasksSet)
{
   MoCoIsAlive = TRUE;
   initCommunications();

   ERROR_MNG(rt_task_spawn(&msgReceiverTask, "MonitoringTask", 0, 99 /*prio*/, 0, MonitoringAgent::messageReceiver, this));

   #if VERBOSE_INFO
   rt_printf("[ Monitoring ] - READY.\n");
   #endif
   rt_print_flush_buffers();
}

MonitoringControlAgent::MonitoringControlAgent(rtTaskInfosStruct _taskInfo,
                  std::vector<end2endDeadlineStruct> e2eDD,
                  std::vector<rtTaskInfosStruct> tasksSet) :
                  MonitoringAgent(_taskInfo, e2eDD, tasksSet) {}

void Agent::initCommunications()
{
   ERROR_MNG(rt_buffer_create(&_buff, MESSAGE_TOPIC_NAME, 20*sizeof(monitoringMsg), B_FIFO));
   ERROR_MNG(rt_event_create(&_event, CHANGE_MODE_EVENT_NAME, 0, EV_PRIO));
   rt_printf("Buffer and Event flag created.\n"); rt_print_flush_buffers();
}

/***********************
* Création des différentes chaines de tâche avec e2e deadline associée
* @params : std::vector<end2endDeadlineStruct> _tcDeadlineStructs
*           liste d' ID de chaines de tâche avec e2e deadline associées
* @returns : /
***********************/
void Agent::setAllChains(std::vector<end2endDeadlineStruct> _tcDeadlineStructs)
{
   #if VERBOSE_INFO
   printf("[ MoCoAgent ] - setting deadlines.\n");
   #endif
   for (auto& tcDeadlineStruct : _tcDeadlineStructs)
   {
      if (tcDeadlineStruct.taskChainID) // if ID = 0 : NRT tasks, not a chain.
      {
         taskChain* tc = new taskChain(tcDeadlineStruct, _stdOut);
         allTaskChain.push_back(*tc);
      }
   }
   allTaskChain.shrink_to_fit();
}

/***********************
* Ajout des tâches critiques aux Tasks Chains
* Les tâches sont parcourues et ajoutées à leur groupe.
* @params : std::vector<rtTaskInfosStruct> _TasksInfos
*           liste des tâches d'entrée.
* @returns : /
***********************/
void Agent::setAllTasks(std::vector<rtTaskInfosStruct> _TasksInfos)
{
   #if VERBOSE_INFO
   printf("[ MoCoAgent ] - setting tasks.\n");
   #endif
///////// JE NE SAIS GERER QU'AVEC UNE UNIQUE CHAINE DE TACHES !!
   for (auto& _taskInfo : _TasksInfos)
   {
      printf(" Adding task %s (%d).\n", _taskInfo.fP.name, _taskInfo.fP.isHRT);
      if ( !_taskInfo.fP.isHRT )
      {
         printf("Adding %s to BE list.\n", _taskInfo.fP.name);
         RT_TASK* _t = new RT_TASK;
         ERROR_MNG(rt_task_bind(_t, _taskInfo.fP.name, TM_NONBLOCK));
         //printInquireInfo(&_t);
         bestEffortTasks.push_back(*_t);
         //break;
      }
      else
      {
         for (auto& chain : allTaskChain)
         {
            if ( _taskInfo.fP.isHRT == chain.chainID)
            {
               rt_printf("Adding %s to HRT chain %d.\n", _taskInfo.fP.name, chain.chainID);
               //taskMonitoringStruct* tms = new taskMonitoringStruct(_taskInfo);
               //tmpTaskChain.push_back(*new taskMonitoringStruct(_taskInfo));
               chain.taskList.push_back(*new taskMonitoringStruct(_taskInfo, chain.logger));
               break;
            }
         }
      }
   }


   std::vector<taskMonitoringStruct> tmpTaskChain;
   // classement des taches par ordre de precedence.
   for (auto& chain : allTaskChain)
   {
      // tmpTaskChain = chain.taskList;
      //chain.taskList.clear();
      uint precedent = 0;
      bool stop = FALSE;
      while (!stop)
      {
         stop = TRUE;
         for (auto&& task : chain.taskList)
         {
            if (task.precedencyID == precedent)
            {
               tmpTaskChain.push_back(task);
               precedent = task.id;
               stop = FALSE;
               break;
            }
         }

      }
      chain.taskList = tmpTaskChain;
      chain.taskList.shrink_to_fit();

      chain.lastTask = &(chain.taskList.back());
      chain.logger->setLogArray(chain.taskList.size() - 1);
      chain.setPrecedencies();

      tmpTaskChain.clear();
   }

   bestEffortTasks.shrink_to_fit();
}


void Agent::executeRun()
{
   while(!EndOfExpe)
   {
      rt_task_wait_period(&overruns);
   }
}


void MonitoringAgent::executeRun()
{
   #if VERBOSE_LOGS
   RTIME _time;
   #endif
   while(!EndOfExpe)
   {
      #if VERBOSE_LOGS
      _time = rt_timer_read();
      #endif
      for (auto& chain : allTaskChain)
      {
         // Execute part //
         if ( chain.checkTaskE2E() && !chain.isAtRisk)
         {
            #if VERBOSE_LOGS
            rt_fprintf(stderr, "%llu ; Monitoring Agent ; %llu\n", _time, rt_timer_read());
            #endif
            chain.isAtRisk = TRUE;
            chain.logger->cptAnticipatedMisses++;
         }
      }

      #if VERBOSE_LOGS
      rt_fprintf(stderr, "%llu ; Monitoring Agent ; %llu\n", _time, rt_timer_read());
      #endif
      rt_task_wait_period(&overruns);
   }

   //rt_task_delete(&msgReceiverTask);
}

void MonitoringControlAgent::executeRun()
{
   //int ret_msg = 0;
   #if VERBOSE_LOGS
   RTIME _time;
   #endif
   while(!EndOfExpe)
   {
      /* ALL MO Co Code IN 1 function
      ret_msg = rt_buffer_read(&_buff, &msg, sizeof(monitoringMsg), TM_NONBLOCK);
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
         case sizeof(monitoringMsg) :
         break;
         //CASES(-ETIMEDOUT, -EINTR, -EINVAL, -EIDRM, -EPERM)
         default :
         rt_fprintf(stderr, "[ MOCOAGENT ] Error on receiving message, code %s [%d].\n", getErrorName(ret_msg), ret_msg);
         break;
      } */
      #if VERBOSE_LOGS
      _time = rt_timer_read();
      #endif
      for (auto& chain : allTaskChain)
      {
         // Execute part //
         if ( chain.checkTaskE2E() && !chain.isAtRisk)
         {
            chain.isAtRisk = TRUE;
            chain.logger->cptAnticipatedMisses++;
            setMode(MODE_OVERLOADED);
         }
      }

      #if VERBOSE_LOGS
      rt_fprintf(stdout,"[MCAgent] ; %llu ; %llu\n", rt_timer_read(), _time);
      #endif
      rt_task_wait_period(&overruns);
   }

   rt_task_delete(&msgReceiverTask);
}

void MonitoringAgent::messageReceiver(void* arg)
{
   MonitoringAgent* mocoAgent = (MonitoringAgent*) arg;
   monitoringMsg msg;
   ERROR_MNG(rt_task_set_periodic(NULL, TM_NOW, TM_INFINITE)); // disable periodicity;

   #if VERBOSE_LOGS
   RTIME _time;
   #endif
   while(TRUE)
   {
      //rt_mutex_acquire(&mocoAgent->_bufMtx, TM_INFINITE);
      int ret = rt_buffer_read(&(mocoAgent->_buff), &msg, sizeof(monitoringMsg), TM_NONBLOCK);
      if (ret == sizeof(monitoringMsg))
      {
         #if VERBOSE_LOGS
         _time = rt_timer_read();
         #endif
         mocoAgent->updateTaskInfo(msg);
         #if VERBOSE_LOGS
         rt_fprintf(stderr,"%llu ; Message Receiver ; %llu\n", _time, rt_timer_read());
         #endif
      }
      //rt_mutex_release(&mocoAgent->_bufMtx);
      //rt_task_sleep()_mSEC(1);
      rt_task_yield();
      //rt_task_wait_period(0);
   }
}

void MonitoringAgent::updateTaskInfo(monitoringMsg msg)
{
   //RT_TASK_INFO curtaskinfo;
   //rt_task_inquire((RT_TASK*) msg.task, &curtaskinfo);
   //cout << "Update from task : " << " ("<< msg.ID << ") - " << msg.isExecuted << " T=" << msg.time/1e6 << endl;
   // printInquireInfo((RT_TASK*) msg.task);
   for (auto& chain : allTaskChain)
   {
      for (auto&& _task : chain.taskList)
      {
         //cout << "Logged chain end at : " << _taskChain->currentEndTime/1.0e6 << endl;
         //cout << "Logged chain start at : " << msg.time/1.0e6 << endl;
         if( msg.ID == _task.id )
         {
            if (_task.addEntry({msg.time, msg.endTime}) )
            { // si toute première tache de la chaine.
               chain.updateStartTime();
            }

            if (msg.ID == chain.lastTask->id)
            { // si toute dernière tache de la chaine.
               if ( chain.unloadChain( msg.endTime) )
               { // si tache complete executee.
                  //cout << "LOGGING CHAIN FROM " << chain.startTime << " TO " << msg.endTime << endl;
                  chain.logger->logChain({chain.startTime, msg.endTime-chain.startTime});
                  chain.updateStartTime();
                  if (chain.isAtRisk)
                  {
                     chain.isAtRisk = FALSE;
                  }
                  //sleep(1);
               }
            }
            return;
         }
      }
   }
}

void MonitoringAgent::updateTaskInfo(monitoringMsg msg)
{
   //RT_TASK_INFO curtaskinfo;
   //rt_task_inquire((RT_TASK*) msg.task, &curtaskinfo);
   //cout << "Update from task : " << " ("<< msg.ID << ") - " << msg.isExecuted << " T=" << msg.time/1e6 << endl;
   // printInquireInfo((RT_TASK*) msg.task);
   for (auto& chain : allTaskChain)
   {
      for (auto&& _task : chain.taskList)
      {
         //cout << "Logged chain end at : " << _taskChain->currentEndTime/1.0e6 << endl;
         //cout << "Logged chain start at : " << msg.time/1.0e6 << endl;
         if( msg.ID == _task.id )
         {
            if (_task.addEntry({msg.time, msg.endTime}) )
            { // si toute première tache de la chaine.
               chain.updateStartTime();
            }

            if (msg.ID == chain.lastTask->id)
            { // si toute dernière tache de la chaine.
               if ( chain.unloadChain( msg.endTime) )
               { // si tache complete executee.
                  //cout << "LOGGING CHAIN FROM " << chain.startTime << " TO " << msg.endTime << endl;
                  chain.logger->logChain({chain.startTime, msg.endTime-chain.startTime});
                  chain.updateStartTime();
                  if (chain.isAtRisk && !EndOfExpe)
                  {
                     chain.isAtRisk = FALSE;
                     setMode(MODE_NOMINAL);
                  }
                  //sleep(1);
               }
            }
            return;
         }
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
void Agent::setMode(uint _newMode)
{
   cout << "[MONITORING & CONTROL AGENT] Change mode to " << ((_newMode&MODE_OVERLOADED)?"OVERLOADED":"NOMINAL") << ". " << endl;
   if (!bestEffortTasks.empty())
   {

      if ((_newMode & MODE_OVERLOADED) && (runtimeMode & MODE_NOMINAL))
      { // Pause Best Effort Tasks if Nominal => OVERLOADED.
          rt_fprintf(stderr,"[MCA] [%ld] - Stopping BE tasks.", rt_timer_read());
         //rt_event_clear(&_event, MODE_NOMINAL, NULL); // => MODE_OVERLOADED.
         for (auto& bestEffortTask : bestEffortTasks)
         {   // Publier message pour dire à stopper
            ERROR_MNG(rt_task_suspend(&bestEffortTask));
         }
         #if VERBOSE_DEBUG
         rt_fprintf(stderr,"[MCA] [%ld] - Stopped BE tasks.", rt_timer_read()); // ==1?"OVERLOADED":"NOMINAL"
         #endif
      }

      else if ((_newMode & MODE_NOMINAL) && (runtimeMode & MODE_OVERLOADED))
      { // resume Best Effort if Overloaded => NOMINAL
         for (auto& bestEffortTask : bestEffortTasks)
         {
            rt_task_resume(&bestEffortTask);
         }
         //rt_event_clear(&_event, MODE_OVERLOADED, NULL);
         //rt_event_signal(&_event, MODE_NOMINAL);
         #if VERBOSE_DEBUG // ==1?"OVERLOADED":"NOMINAL"
         rt_fprintf(stderr,"[MCA] [%ld] - Re-started BE tasks.", rt_timer_read());
         #endif
      }
      runtimeMode = _newMode;
      #if VERBOSE_DEBUG // ==1?"OVERLOADED":"NOMINAL"
      rt_fprintf(stderr,"=> MoCoAgent Triggered to mode %s !\n",((_newMode&MODE_OVERLOADED)?"OVERLOADED":"NOMINAL"));
      #endif
   }
}

void Agent::saveData()
{
   string file = _stdOut; // conversion to string

   //rt_task_delete(&msgReceiverTask);
   /*RT_TASK_INFO cti;
   if (rt_task_inquire(NULL, &cti) == 0)
   {
      std::ofstream outputFileResume;
      outputFileResume.open (file + RESUME_FILE, std::ios::app);    // TO APPEND :  //,ios_base::app);
      outputFileResume << "\n Monitoring and Control Agent Stats : " << cti.stat.xtime/1.0e6 << " ms in primary mode.\n"
                    << "        Timeouts - " << cti.stat.timeout << " (" << overruns << ")\n"
                    << "   Mode Switches - " << cti.stat.msw << "\n"
                    << "Context Switches - " << cti.stat.csw << "\n"
                    << "Cobalt Sys calls - " << cti.stat.xsc
                    << endl;
   outputFileResume.close();
   }
*/
   int nameMaxSize = 8;
   if (MoCoIsAlive)
   {
      if (!allTaskChain.empty())
      {
         for (auto& chain : allTaskChain)
         {
            int sizeName = strlen(chain.name);
            if (sizeName > nameMaxSize) nameMaxSize = sizeName;
         }

         std::ofstream outputFileChainData;
         outputFileChainData.open (file + CHAIN_FILE);    // TO APPEND :  //,ios_base::app);

         outputFileChainData << std::setw(15)           << "timestamp" << " ; "
                              << std::setw(nameMaxSize) << "name"      << " ; "
                              << std::setw(2)           << "ID"        << " ; "
                              << std::setw(10)          << "deadline"  << " ; "
                              << std::setw(10)          << "duration"  ;



         for (auto& _taskChain : allTaskChain)
         {
            for (uint j = 0; j < _taskChain.taskList.size() -1; j++)
            {
               outputFileChainData << " ; " << " rWCETs_t" << j;
            }
            outputFileChainData << endl;

            outputFileChainData.close();
            rt_fprintf(stdout, "[ %llu ] - SAVING CHAIN %s DATA.\n", rt_timer_read(), _taskChain.name);
            rt_print_flush_buffers();
            _taskChain.logger->saveData(nameMaxSize);
         }
      }
      else cerr << "WOAW !! Chain set is empty !!" << endl;
   }

}
/***********************
* Fonction de débug pour afficher
* les informations de toutes les tâches reçues.
* @params : [ systemRTInfo sInfos ]
* @returns : cout
***********************/
void Agent::displaySystemInfo(std::vector<end2endDeadlineStruct> _e2eDD,
                                 std::vector<rtTaskInfosStruct> _tasksSet)
{
   #if VERBOSE_ASK
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

void Agent::displayChains()
{
   #if VERBOSE_INFO
   cout << "Displaying MoCoAgent Database : " << "\n";
   for (auto& chain : allTaskChain)
   { // Print chain params
      chain.displayTasks();
   }
   #endif
}

//////////////////////////////////////////
/////////// TASK CHAIN CLASS /////////////
taskChain::taskChain(end2endDeadlineStruct _e2eInfos, string outfile)
{
   strcpy(name, _e2eInfos.name);
   chainID = _e2eInfos.taskChainID;
   end2endDeadline = _e2eInfos.deadline;
   logger = new ChainDataLogger(_e2eInfos, outfile);
   startTime = 0;
}

void taskChain::setPrecedencies()
{
   unsigned long macroPeriod = 0;
   uint bufsize = 0;

   for (auto&& task : taskList)
   {
      macroPeriod += task.deadline;
   }

   for (auto&& task_i : taskList)
   {
      bufsize = 20; //* macroPeriod / task_i.deadline;
      if (task_i.precedencyID == 0)
      {
         task_i.setChainInfos(bufsize, NULL);

      }
      else for (auto&& task_k : taskList)
      {
         if (task_i.precedencyID == task_k.id)
            task_i.setChainInfos(bufsize, &task_k);
      }
   }

}

/*
bool taskChain::checkPrecedency(uint _id)
{ // Attention ! Précédence entre tâche géré que avec une tâche antérieure..!
   bool isOkay = FALSE;
   if (_id == 0) return TRUE;
   else for (auto& task : taskList)
   { // Plus simple à coder..!
      isOkay = isOkay || ((task.id == _id) && task.getState());
   }
   return isOkay;
}
*/
/***********************
* Fonction limite respect de deadline
* @params : [ systemRTInfo sInfos ]
* @returns : 1 if OK ; 0 if RISK
***********************/
bool taskChain::checkTaskE2E()
{
   bool miss = FALSE;
   if (startTime != 0)
   {
      RTIME execTime = getExecutionTime();
      //rt_printf("Exec Time = %lu.\n", execTime); rt_print_flush_buffers();
      RTIME remTime = getRemWCET();
      //rt_printf("Remaining Time = %lu.\n", remTime); rt_print_flush_buffers();
      miss = ( execTime + Wmax + t_RT + remTime > end2endDeadline ); // TOUT EN ns !!
      //cout << "Exec Time : " << execTime/1e6 << " | Rem Time : " << remTime/1e6 << " | Deadline : " << end2endDeadline/1e6 << endl;
   }
   //if (miss) cptAnticipatedMisses++;
   return (miss);
}

bool taskChain::unloadChain(RTIME endOfChain)
{
   return lastTask->emptyPrecedency( 0, endOfChain);
}

void taskChain::updateStartTime()
{
   startTime = taskList.begin()->getState().start;
   for (unsigned i = 1; i < taskList.size(); i++)
   {
      taskList.at(i).emptyUntil(startTime);
   }
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
   for (auto& task : taskList)
   {
      ExecTimes state = task.getState();
      //cout << state << "-";
      if (!state.start)
      {
         return task.rwcet;
      }
   }
   return 0;
}

void taskChain::displayTasks()
{
  #if VERBOSE_INFO
  cout << "|- Chain #" << chainID << " - " << name
      << "  |- Deadline: " << end2endDeadline /1.0e6 << " ms."
      << endl;
  for (auto &_task : taskList)
  { // Print task Params
     _task.displayInfos();
  }
  #endif
}

/////////////////////////////////////////////////
//////////// TASK MONITORING STRUCTURE //////////
/////////////////////////////////////////////////
/***********************
* Recoit la liste des tasksInfos lues depuis l'INPUT.txt
* Le converti avec uniquement les informations nécessaires pour le MoCoAgent
* @params : vector<rtTaskInfosStruct> rtTasks
* @returns : [ vector<taskMonitoringStruct> taskList ]
***********************/
taskMonitoringStruct::taskMonitoringStruct(rtTaskInfosStruct _taskInfos, ChainDataLogger* dataLogger)
{
   ERROR_MNG(rt_task_bind(&xenoTask, _taskInfos.fP.name, TM_NONBLOCK));
   deadline = _taskInfos.rtP.periodicity;
   rwcet = _taskInfos.fP.wcet;
   id = _taskInfos.fP.id;
   precedencyID = _taskInfos.fP.prec;


   oldestElement = 0;
   newestElement = 0;
   precedentTask = NULL;
   logger = dataLogger;

/* useless
   setState(FALSE);
   //endTime = 0;
   #if USE_MUTEX
   char mutexName[38];
   strcpy(mutexName, _taskInfos.fP.name);
   strncat(mutexName, "_StMTX", 6);
   rt_mutex_create(&mtx_taskStatus, mutexName);
   #endif
*/
}

void taskMonitoringStruct::setChainInfos(int bufsize, taskMonitoringStruct* prec)
{
   execLogs = new ExecTimes[bufsize]; // amount to define.
   maxPendingChains = bufsize;
   precedentTask = prec;
}

/* addEntry(ExecTimes times)
/  Ajoute une valeur d'execution de la tache dans la table
/  si il n'y avait plus de log d'execution dans la table,
/  renvoyer qu'il faut mettre à jour la date de début d'Execution
/  de la chaine de tâche monitorer.
*/
bool taskMonitoringStruct::addEntry(ExecTimes times)
{
   bool updateFollowedChain = FALSE;
   if (!precedencyID && newestElement == oldestElement ) updateFollowedChain = TRUE;
   execLogs[newestElement] = times;
   newestElement++;
   newestElement = (newestElement == maxPendingChains ? 0 : newestElement); // comme un modulo
   if (newestElement == oldestElement)
   {
      rt_fprintf(stderr, "[ %llu ][ %d ] - Too much logs ! Discarding.", rt_timer_read(), id);
      newestElement = (newestElement ? (newestElement - 1) : (maxPendingChains-1));
   }

   /*
   uint i = oldestElement;
   cout << "Task #" << id << " : ";
   while (i != newestElement)
   {
      cout << "[" << execLogs[i].start-execLogs[oldestElement].start << "/" << execLogs[i].end-execLogs[oldestElement].start << "] | ";
      i++;
      i = (i == maxPendingChains ? 0 : i); // comme un modulo
   }
   cout << oldestElement << "/" << newestElement << endl;
   cout.flush();
   */
   return updateFollowedChain;
}

ExecTimes taskMonitoringStruct::emptyUntil(RTIME limitTime)
{
   //cout << "Emptying precedency of " << id << " => limitTime=" << limitTime << " ; " << oldestElement << "/" << newestElement << " ; ";

   ExecTimes _time = {0,0};
   // while there is still elements in the array AND
   // the time values are valid if there's a time value limit.

   while ( ( oldestElement != newestElement ) &&
           ( (execLogs[oldestElement].end < limitTime) || !limitTime) )
   {
      _time = execLogs[oldestElement]; // save last removed element times
      oldestElement++; // remove element
      oldestElement = (oldestElement == maxPendingChains ? 0 : oldestElement); // comme un modulo
   }

   //cout << "->" << oldestElement << "/" << newestElement << endl;
   return _time;
}

// ENTREE : start de la tâche suivante de chaine
// SORTIE : start time valide (le + ancien) de la première tache de chaine
bool taskMonitoringStruct::emptyPrecedency( RTIME limitTime, RTIME endOfChain)
{
   ExecTimes _time = emptyUntil(limitTime);

   if (_time.start == 0) return FALSE; // no tasks removed from buffer, stop recursion

   bool result = TRUE;

   if ( precedencyID ) // not the end of recursion
      result = precedentTask->emptyPrecedency(_time.start, endOfChain);

   if (result && limitTime) // uniquement si on est pas sur la toute dernière tâche
   {
      //cout << "Logging WCET from #" << id << " to end. D=" << endOfChain - _time.end << endl;
      // log du WCET d'etat de chaine quand on en est à CETTE tache qui est terminee.
      // Pour t2 :  [t1] -> [t2]( -> [t3] -> [t4])
      //                       ´|`__> WCET de fin de t2 à fin de t4.
      //rt_printf("LOG @%d for #%d", logger, id);
      logger->logWCET(endOfChain - _time.end);


   }

   return  result;
}

ExecTimes taskMonitoringStruct::getState(RTIME limitTime)
{
   // si aucun élément dans le tableau
   if (oldestElement == newestElement) return {0,0};

   #if USE_MUTEX
      rt_mutex_acquire(&mtx_taskStatus, TM_INFINITE);
   #endif

   uint i = oldestElement;
   while ( ( i != newestElement ) &&
           (execLogs[i].start < limitTime) )
   {
      i++; // next element
      i = (i == maxPendingChains ? 0 : i); // comme un modulo
   }
      return execLogs[i];

   #if USE_MUTEX
   rt_mutex_release(&mtx_taskStatus);
   #endif


}

void taskMonitoringStruct::displayInfos()
{
   RT_TASK_INFO xenoInfos;
   rt_task_inquire(&xenoTask, &xenoInfos);
   cout << "  |- Task    : "  << xenoInfos.name     << "\n"
        << "   ‾|- ID      : " << id               << "\n"
        << "     |- Prec. ID: " << precedencyID  << "\n"
        << "     |- Deadline: " << deadline /1.0e6  << "\n"
        << "     |- WCET    : " << rwcet /1.0e6     << "\n"
        << "     |- Priority: " << xenoInfos.prio      //   << "\n"
        << endl;
}

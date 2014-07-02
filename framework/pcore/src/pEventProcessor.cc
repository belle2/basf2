/*
 * pEventProcessor.cc
 *
 *  Created on: Oct 26, 2010
 *      Author: R.Itoh, IPNS, KEK
 *  Updated : Jul. 25, 2011
 */

#include <framework/pcore/pEventProcessor.h>
#include <framework/pcore/EvtMessage.h>
#include <framework/pcore/ProcHandler.h>
#include <framework/pcore/RingBuffer.h>
#include <framework/pcore/RxModule.h>
#include <framework/pcore/TxModule.h>
#include <framework/pcore/HistModule.h>

#include <framework/core/Environment.h>
#include <framework/logging/LogSystem.h>

#include <TROOT.h>

#include <signal.h>
#include <sys/wait.h>
#include <stdlib.h>


using namespace std;
using namespace Belle2;

/** PID of sub-process killed by some signal. 0 if none. */
static int s_PIDofKilledChild = 0;

static pEventProcessor* g_pEventProcessor = NULL;

static void signalHandler(int signal)
{
  //signal handlers are called asynchronously, making many standard functions (including output) dangerous
  //write() is, however, safe, so we'll use that to write to stderr.
  if (signal == SIGSEGV) {
    EventProcessor::writeToStdErr("\nProcess died with SIGSEGV (Segmentation fault).\n");
    abort();
  } else if (signal == SIGINT) {
    g_pEventProcessor->gotSigINT();
  } else if (signal == SIGCHLD) {
    if (s_PIDofKilledChild != 0)
      return; //only once

    //which child died?
    int status;
    pid_t pid = waitpid(-1, &status, WNOHANG);
    //B2WARNING("child died, pid " << pid << ", status " << status);
    if (pid <= 0 || !WIFSIGNALED(status))
      return; //process exited normally

    //ok, it died because of some signal
    s_PIDofKilledChild = pid;
    //int termsig = WTERMSIG(status);

    EventProcessor::writeToStdErr("\nOne of our child processes died, stopping execution...\n");

    //pid=0: send signal to every process in our progress group (ourselves + children)
    kill(0, SIGTERM);
  }
}

pEventProcessor::pEventProcessor() : EventProcessor(),
  m_procHandler(new ProcHandler()),
  m_histoManagerFound(false),
  m_enableRBClearing(true)
{
  g_pEventProcessor = this;
}


pEventProcessor::~pEventProcessor()
{
  delete m_procHandler;
}

void pEventProcessor::sendTerminationMessage(RingBuffer* rb)
{
  m_enableRBClearing = false;
  EvtMessage term(NULL, 0, MSG_TERMINATE);
  while (rb->insq((int*)term.buffer(), term.paddedSize()) < 0) {
    usleep(20);
  }
  m_enableRBClearing = true;
}


void pEventProcessor::gotSigINT()
{
  static int numSigInts = 0;
  if (numSigInts == 0) {
    EventProcessor::writeToStdErr("\nStopping basf2 after all events in buffer are processed. Press Ctrl+C a second time to discard half-processed events.\n");
    //SIGINT is handled independently by input process, so we don't need to do anything special
  } else if (m_enableRBClearing) {
    m_enableRBClearing = false;
    EventProcessor::writeToStdErr("\nDiscarding pending events for quicker termination... \n");
    //clear ringbuffers and add terminate message
    const int numProcesses = Environment::Instance().getNumberProcesses();
    for (RingBuffer * rb : m_rbinlist) {
      rb->clear();
      for (int i = 0; i < numProcesses; i++) {
        sendTerminationMessage(rb);
      }
    }
    for (RingBuffer * rb : m_rboutlist) {
      rb->clear();
      sendTerminationMessage(rb);
    }
    m_enableRBClearing = true;
  } else {
    //our process is currently using the RingBuffers, so don't do anything
  }
  numSigInts++;
}

void pEventProcessor::clearFileList()
{
  //B2WARNING("list of files: " << gROOT->GetListOfFiles()->GetEntries());
  //clear list, but don't actually delete the objects
  gROOT->GetListOfFiles()->Clear("nodelete");
}


void pEventProcessor::process(PathPtr spath, long maxEvent)
{
  if (spath->getModules().size() == 0) return;

  const int numProcesses = Environment::Instance().getNumberProcesses();

  //Check whether the number of events was set via command line argument
  int numEventsArgument = Environment::Instance().getNumberEventsOverride();
  if ((numEventsArgument > 0) && ((maxEvent == 0) || (maxEvent > numEventsArgument))) {
    maxEvent = numEventsArgument;
  }

  if (numProcesses == 0) {
    B2FATAL("pEventProcessor::process() called for serial processing! Most likely a bug in Framework.");
    return;
  }

  // 1. Analyze start path and split into parallel paths
  analyze_path(spath); //also inserts Rx/Tx modules into path (sets up IPC structures)


  dump_path("Input Path ", m_inpathlist[0]);
  for (unsigned int i = 0; i < m_mainpathlist.size(); i++) {
    dump_path("Main Path ", m_mainpathlist[i]);
  }
  for (unsigned int i = 0; i < m_outpathlist.size(); i++) {
    dump_path("Output Path ", m_outpathlist[i]);
  }
  if (m_mainpathlist.empty()) {
    B2WARNING("Cannot run any modules in parallel (no c_ParallelProcessingCertified flag), falling back to single-core mode.");
    EventProcessor::process(spath, maxEvent);
    return;
  }


  // 2. Initialization
  ModulePtrList modulelist = spath->buildModulePathList();;
  //  dump_modules ( "full : ", modulelist );
  ModulePtrList initGlobally = getModulesWithoutFlag(modulelist, Module::c_InternalSerializer);
  dump_modules("processInitialize : ", initGlobally);
  processInitialize(initGlobally);

  ModulePtrList terminateGlobally = getModulesWithFlag(modulelist, Module::c_TerminateInAllProcesses);

  //Don't start processing in case of no master module
  if (!m_master) {
    B2ERROR("There is no module that provides event and run numbers. You must either add the EventInfoSetter module to your path, or, if using an input module, read EventMetaData objects from file.");
  }

  //Check if errors appeared. If yes, don't start the event processing.
  int numLogError = LogSystem::Instance().getMessageCounter(LogConfig::c_Error);
  if (numLogError != 0) {
    B2FATAL(numLogError << " ERROR(S) occurred! The processing of events will not be started.");
  }

  //disable ROOT's management of TFiles
  clearFileList();

  //If we crash after forking, ROOTs SIGSEGV handler could potentially use huge amounts of memory (scales with numProcesses)
  /*
  if (signal(SIGSEGV, signalHandler) == SIG_ERR) {
    B2FATAL("Cannot setup SIGSEGV signal handler\n");
  }
  */

  //Path for current process
  PathPtr localPath;

  // 3. Fork input path
  m_procHandler->startInputProcess();
  if (m_procHandler->isInputProcess()) {   // In input process
    localPath = m_inpathlist[0];
  }

  if (localPath == nullptr) { //not forked yet
    // for all processes except the input process (already started by now)
    // ignore SIGINT so we can do our own handling to ensure safe termination.
    if (signal(SIGINT, SIG_IGN) == SIG_ERR) {
      B2FATAL("Cannot ignore SIGINT signal handler\n");
    }

    // 4. Fork output paths
    int nout = 0;
    for (PathPtr & outpath : m_outpathlist) {
      if ((outpath->getModules()).size() > 0) {
        m_procHandler->startOutputProcess(nout);
        if (m_procHandler->isOutputProcess()) {   // In output process
          localPath = outpath;
          m_master = localPath->getModules().begin()->get(); //set Rx as master
        }
        nout++;
      }
    }
  }

  if (localPath == nullptr) { //not forked yet
    // 5. Fork out main path (parallel part)
    fflush(stdout);
    m_procHandler->startEventProcesses(numProcesses);
    if (m_procHandler->isEventProcess()) {
      localPath = m_mainpathlist[m_mainpathlist.size() - 1];
      m_master = localPath->getModules().begin()->get(); //set Rx as master
    }
  }

  //we're one of the forked processes
  if (localPath != nullptr) {
    ModulePtrList localModules = localPath->buildModulePathList();
    ModulePtrList procinitmodules = getModulesWithFlag(localModules, Module::c_InternalSerializer);
    //dump_modules("processInitialize for ", procinitmodules);
    processInitialize(procinitmodules);

    if (m_procHandler->isInputProcess())
      setupSignalHandler();

    processCore(localPath, localModules, maxEvent);
    prependModulesIfNotPresent(&localModules, terminateGlobally);
    processTerminate(localModules);

    B2INFO(m_procHandler->getProcessName() << " process finished.");
    exit(0);
  } else { // 6. Framework process

    //ignore some signals for framework (mother) process, so only child processes will handle them
    //once they are finished, the framework process will clean up IPC structures
    if (signal(SIGTERM, SIG_IGN) == SIG_ERR) {
      B2FATAL("Cannot ignore SIGTERM signal handler for main process\n");
    }
    if (signal(SIGQUIT, SIG_IGN) == SIG_ERR) {
      B2FATAL("Cannot ignore SIGQUIT signal handler for main process\n");
    }
    //If our children are killed, we want to know. (especially since this may happen in any order)
    if (signal(SIGCHLD, signalHandler) == SIG_ERR) {
      B2FATAL("Cannot setup SIGCHLD signal handler\n");
    }
    //first time: just print a message, afterwards tries to terminate more quickly
    if (signal(SIGINT, signalHandler) == SIG_ERR) {
      B2FATAL("Cannot set SIGINT signal handler\n");
    }

    // 6.0 Build End of data message
    // 6.1 Wait for input path to terminate
    m_procHandler->waitForInputProcesses();
    // 6.2 Send termination to event processes
    for (int i = 0; i < numProcesses; i++) {
      for (RingBuffer * rb : m_rbinlist) {
        sendTerminationMessage(rb);
      }
    }
    // 6.3 Wait for event processes to terminate
    m_procHandler->waitForEventProcesses();
    // 6.4 Send termination to output processes
    for (RingBuffer * rb : m_rboutlist) {
      sendTerminationMessage(rb);
    }
    m_procHandler->waitForOutputProcesses();
    B2INFO("All processes completed");

    //finished, disable handler again
    if (signal(SIGINT, SIG_IGN) == SIG_ERR) {
      B2FATAL("Cannot ignore SIGINT signal handler\n");
    }

    HistModule::mergeFiles();

    // 6.5 Remove all ring buffers
    for (RingBuffer * rb : m_rbinlist)
      delete rb;
    for (RingBuffer * rb : m_rboutlist)
      delete rb;

    processTerminate(terminateGlobally);

    B2INFO("Global process: completed");

    //did anything bad happen?
    if (s_PIDofKilledChild != 0) {
      //fatal, so we get appropriate return code (IPC cleanup was already done)
      B2FATAL("Execution stopped, sub-process with PID " << s_PIDofKilledChild << " killed by signal.");
    }

  }
}


void pEventProcessor::analyze_path(const PathPtr& path, Module* inmod, int cstate)
{

  const ModulePtrList& modlist = path->getModules();
  ModulePtrList::const_iterator iter;

  ModulePtrList inlist;
  ModulePtrList mainlist;
  ModulePtrList outlist;

  // Set state
  int state = 0;
  if (inmod != NULL) {
    state = 1;
    B2DEBUG(0, "Analyze Path: called with condition module");
  }

  int nrbin = 0;
  int nrbout = 0;
  // Loop over modules on the path
  for (iter = modlist.begin(); iter != modlist.end(); ++iter) {
    ModulePtr modptr = *iter;
    Module* module = iter->get();
    // Check Module property
    if (state == 0) {   // non-parallel modules for input
      /*
      if ( iter == modlist.begin() &&
      module->hasProperties ( Module::c_HistogramManager ) ) {
      if ( module->hasProperties ( Module::c_HistogramManager ) ) {
      m_histoManagerFound = true;
      m_histoman = *iter;  // Register histomanager module if exists
      }
      */
      if (!module->hasProperties(Module::c_ParallelProcessingCertified)) {
        inlist.push_back(modptr);
        B2DEBUG(0, "Analyze Path : state=0, Module " << module->getName() << " added to inlist");
        if (module->hasCondition()) {
          B2DEBUG(0, "Analyze Path : Condition detected, recursive call");
          PathPtr next = module->getConditionPath();
          analyze_path(next, module, state);
        }
        if (module->hasProperties(Module::c_HistogramManager)) {
          // Initialize histogram manager if found in the path
          m_histoManagerFound = true;
          m_histoman = *iter;
          module->initialize();
        }
      } else {
        state = 1;
        if (!(inlist.empty())) {
          // Create RingBuffer
          char* inrbname = getenv("BASF2_RBIN");
          RingBuffer* rbuf;
          if (inrbname == NULL) {
            rbuf = new RingBuffer();
          } else {
            char rbname[256];
            sprintf(rbname, "%s%d", inrbname, nrbin);
            rbuf = new RingBuffer(rbname, RingBuffer::c_DefaultSize);
          }
          m_rbinlist.push_back(rbuf);
          nrbin++;
          // Insert Tx at the end of current path
          ModulePtr txptr(new TxModule(rbuf));
          inlist.push_back(txptr);
          // Inserv Rx at the top of next path
          ModulePtr rxptr(new RxModule(rbuf));
          mainlist.push_back(rxptr);
          B2DEBUG(0, "Analyze Path : state=0->1, Tx and Rx are inserted");
          if (m_histoManagerFound) {
            mainlist.push_back(m_histoman);
            B2DEBUG(0, "Analyze Path : state=0->1, HistoMan is inserted");
          }
        }
        mainlist.push_back(modptr);
        B2DEBUG(0, "Analyze Path : state=0->1, Module " << module->getName() << " added to mainlist");
        if (module->hasCondition()) {
          B2DEBUG(0, "Analyze Path : Condition detected, recursive call");
          PathPtr next = module->getConditionPath();
          analyze_path(next, module, state);
        }
      }
    } else if (state == 1) {
      if (module->hasProperties(Module::c_ParallelProcessingCertified)) {
        mainlist.push_back(modptr);
        B2DEBUG(0, "Analyze Path : state=1, Module " << module->getName() << " added to mainlist");
        if (module->hasCondition()) {
          B2DEBUG(0, "Analyze Path : Condition detected, recursive call");
          PathPtr next = module->getConditionPath();
          analyze_path(next, module, state);
        }
      } else {
        state = 2;
        if (!(mainlist.empty()) || (cstate == 1 && mainlist.empty())) {
          // Create RingBuffer
          //          RingBuffer* rbuf = new RingBuffer();
          char* inrbname = getenv("BASF2_RBOUT");
          RingBuffer* rbuf;
          if (inrbname == NULL) {
            rbuf = new RingBuffer();
          } else {
            char rbname[256];
            sprintf(rbname, "%s%d", inrbname, nrbout);
            rbuf = new RingBuffer(rbname, RingBuffer::c_DefaultSize);
          }
          m_rboutlist.push_back(rbuf);
          nrbout++;
          // Insert Tx at the end of current path
          ModulePtr txptr(new TxModule(rbuf));
          mainlist.push_back(txptr);
          // Insert Rx at the top of next path
          ModulePtr rxptr(new RxModule(rbuf));
          outlist.push_back(rxptr);
          B2DEBUG(0, "Analyze Path : state=1->2, Tx and Rx are inserted");
          if (m_histoManagerFound) {
            outlist.push_back(m_histoman);
            B2DEBUG(0, "Analyze Path : state=1->2, HistoMan is inserted");
          }
        }
        outlist.push_back(modptr);
        B2DEBUG(0, "Analyze Path : state=1->2, Module " << module->getName() << " added to outlist");
        if (module->hasCondition()) {
          B2DEBUG(0, "Analyze Path : Condition detected, recursive call");
          PathPtr next = module->getConditionPath();
          analyze_path(next, module, state);
        }
      }
    } else if (state == 2) {
      // Modules after non-parallel module are all placed in output path.
      B2DEBUG(0, "Analyze Path : state=2, Module " << module->getName() << " added to outlist");
      outlist.push_back(modptr);
      if (module->hasCondition()) {
        B2DEBUG(0, "Analyze Path : Condition detected, recursive call");
        PathPtr next = module->getConditionPath();
        analyze_path(next, module, state);
      }
    }
  }
  B2DEBUG(100, "Analyze Path : mainlist size = " << mainlist.size());
  B2DEBUG(100, "Analyze Path : inlist size = " << mainlist.size());
  B2DEBUG(100, "Analyze Path : outlist size = " << mainlist.size());

  PathPtr inpath(new Path);
  if (!inlist.empty()) {
    inpath->putModules(inlist);
    m_inpathlist.push_back(inpath);
  }
  PathPtr bodypath(new Path);
  if (!mainlist.empty()) {
    bodypath->putModules(mainlist);
    m_mainpathlist.push_back(bodypath);
  }
  PathPtr outpath(new Path);
  if (!outlist.empty()) {
    outpath->putModules(outlist);
    m_outpathlist.push_back(outpath);
  }

  // Set new condition path to condition module
  if (inmod != NULL && !mainlist.empty()) {
    inmod->setConditionPath(bodypath);
  }
}

void pEventProcessor::dump_path(const std::string title, PathPtr path)
{
  const ModulePtrList& modlist = path->getModules();
  ModulePtrList::const_iterator it;
  std::ostringstream strbuf;
  strbuf << title << "(" << path.get() << ") : ";
  for (it = modlist.begin(); it != modlist.end(); ++it) {
    const Module* module = it->get();
    strbuf << module->getName();
    if (module->hasCondition()) {
      PathPtr condpath = module->getConditionPath();
      strbuf << "[->" << condpath.get() << "] ";
    }
    if (*it != modlist.back())
      strbuf << " -> ";
  }
  B2INFO(strbuf.str());
}

void pEventProcessor::dump_modules(const std::string title, const ModulePtrList modlist)
{
  ModulePtrList::const_iterator it;
  std::ostringstream strbuf;
  strbuf << title << " : ";
  for (it = modlist.begin(); it != modlist.end(); ++it) {
    const Module* module = it->get();
    strbuf << module->getName();
    if (module->hasCondition()) {
      PathPtr condpath = module->getConditionPath();
      strbuf << "[->" << condpath.get() << "] ";
    }
    if (*it != modlist.back())
      strbuf << " -> ";
  }
  B2INFO(strbuf.str());
}


ModulePtrList pEventProcessor::getModulesWithFlag(const ModulePtrList& modules, Module::EModulePropFlags flag)
{
  ModulePtrList tmpModuleList;
  ModulePtrList::const_iterator listIter;

  for (const ModulePtr & m : modules) {
    if (m->hasProperties(flag))
      tmpModuleList.push_back(m);
  }

  return tmpModuleList;
}
ModulePtrList pEventProcessor::getModulesWithoutFlag(const ModulePtrList& modules, Module::EModulePropFlags flag)
{
  ModulePtrList tmpModuleList;
  ModulePtrList::const_iterator listIter;

  for (const ModulePtr & m : modules) {
    if (!m->hasProperties(flag))
      tmpModuleList.push_back(m);
  }
  return tmpModuleList;
}

void pEventProcessor::prependModulesIfNotPresent(ModulePtrList* modules, const ModulePtrList& prependModules)
{
  for (const ModulePtr & m : prependModules) {
    if (std::find(modules->begin(), modules->end(), m) == modules->end()) { //not present
      modules->push_front(m);
    }
  }
}

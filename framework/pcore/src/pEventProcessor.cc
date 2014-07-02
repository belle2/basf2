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
  m_histoman(nullptr),
  m_enableRBClearing(true)
{
  g_pEventProcessor = this;
}


pEventProcessor::~pEventProcessor()
{
  delete m_procHandler;
}

void pEventProcessor::sendTerminationMessage(boost::shared_ptr<RingBuffer> rb)
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
    for (auto rb : m_rbinlist) {
      rb->clear();
      for (int i = 0; i < numProcesses; i++) {
        sendTerminationMessage(rb);
      }
    }
    for (auto rb : m_rboutlist) {
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
  analyzePath(spath);


  if (!m_inpathlist.empty())
    B2INFO("Input Path " << m_inpathlist[0]->getPathString());
  if (!m_mainpathlist.empty())
    B2INFO("Main Path " << m_mainpathlist[0]->getPathString());
  if (!m_outpathlist.empty())
    B2INFO("Output Path " << m_outpathlist[0]->getPathString());
  if (m_mainpathlist.empty()) {
    B2WARNING("Cannot run any modules in parallel (no c_ParallelProcessingCertified flag), falling back to single-core mode.");
    EventProcessor::process(spath, maxEvent);
    return;
  }

  //inserts Rx/Tx modules into path (sets up IPC structures)
  preparePaths();

  // 2. Initialization
  ModulePtrList modulelist = spath->buildModulePathList();;
  ModulePtrList initGlobally = getModulesWithoutFlag(modulelist, Module::c_InternalSerializer);
  //dump_modules("Initializing globally: ", initGlobally);
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
      for (auto rb : m_rbinlist) {
        sendTerminationMessage(rb);
      }
    }
    // 6.3 Wait for event processes to terminate
    m_procHandler->waitForEventProcesses();
    // 6.4 Send termination to output processes
    for (auto rb : m_rboutlist) {
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
    m_rbinlist.clear();
    m_rboutlist.clear();

    processTerminate(terminateGlobally);

    B2INFO("Global process: completed");

    //did anything bad happen?
    if (s_PIDofKilledChild != 0) {
      //fatal, so we get appropriate return code (IPC cleanup was already done)
      B2FATAL("Execution stopped, sub-process with PID " << s_PIDofKilledChild << " killed by signal.");
    }

  }
}

void pEventProcessor::analyzePath(const PathPtr& path)
{
  //TODO this should either be properly generalized so it can deal with multiple parallel sections,
  //or one can simply get rid of the different lists for paths...
  PathPtr inpath(new Path);
  PathPtr mainpath(new Path);
  PathPtr outpath(new Path);

  int stage = 0; //0: in, 1: event/main, 2: out
  for (const ModulePtr & module : path->getModules()) {
    //TODO also check conditional path, or have setConditionPath() update module flag...
    const bool hasParallelFlag = module->hasProperties(Module::c_ParallelProcessingCertified);

    //update stage?
    if ((stage == 0 and hasParallelFlag) or (stage == 1 and !hasParallelFlag))
      stage++;

    if (stage == 0) { //fill input path
      inpath->addModule(module);

      if (module->hasProperties(Module::c_HistogramManager)) {
        // Initialize histogram manager if found in the path
        m_histoman = module;

        //add histoman to other paths
        mainpath->addModule(m_histoman);
        outpath->addModule(m_histoman);
      }
    }

    if (stage == 1)
      mainpath->addModule(module);
    if (stage == 2)
      outpath->addModule(module);
  }

  if (!inpath->isEmpty())
    m_inpathlist.push_back(inpath);
  if (!mainpath->isEmpty())
    m_mainpathlist.push_back(mainpath);
  if (!outpath->isEmpty())
    m_outpathlist.push_back(outpath);
}

RingBuffer* pEventProcessor::connectViaRingBuffer(const char* name, PathPtr a, PathPtr& b)
{
  //create ringbuffers and add rx/tx where needed
  char* inrbname = getenv(name);
  RingBuffer* rbuf;
  if (inrbname == NULL) {
    rbuf = new RingBuffer();
  } else {
    char rbname[256];
    sprintf(rbname, "%s%d", inrbname, 0); //currently at most one input, one output buffer
    rbuf = new RingBuffer(rbname, RingBuffer::c_DefaultSize);
  }

  // Insert Tx at the end of current path
  ModulePtr txptr(new TxModule(rbuf));
  a->addModule(txptr);
  // Insert Rx at beginning of next path
  ModulePtr rxptr(new RxModule(rbuf));
  PathPtr newB(new Path());
  newB->addModule(rxptr);
  newB->addPath(b);
  b.swap(newB);

  return rbuf;
}

void pEventProcessor::preparePaths()
{
  if (m_histoman) {
    m_histoman->initialize();
  }
  if (m_mainpathlist.empty())
    return; //we'll fall back to single-core

  if (!m_inpathlist.empty())
    m_rbinlist.emplace_back(connectViaRingBuffer("BASF2_RBIN", m_inpathlist[0], m_mainpathlist[0]));
  if (!m_outpathlist.empty())
    m_rboutlist.emplace_back(connectViaRingBuffer("BASF2_RBOUT", m_mainpathlist[0], m_outpathlist[0]));

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

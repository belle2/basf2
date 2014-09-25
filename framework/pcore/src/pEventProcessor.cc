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
#include <framework/pcore/DataStoreStreamer.h>

#include <framework/core/Environment.h>
#include <framework/logging/LogSystem.h>

#include <TROOT.h>

#include <signal.h>
#include <sys/wait.h>
#include <cstdlib>


using namespace std;
using namespace Belle2;

namespace {
  /** PID of sub-process killed by some signal. 0 if none. */
  static int s_PIDofKilledChild = 0;

  static pEventProcessor* g_pEventProcessor = NULL;

  void cleanupIPC()
  {
    if (g_pEventProcessor)
      g_pEventProcessor->cleanup();
  }

  static void signalHandler(int signal)
  {
    //signal handlers are called asynchronously, making many standard functions (including output) dangerous
    //write() is, however, safe, so we'll use that to write to stderr.
    if (signal == SIGSEGV) {
      EventProcessor::writeToStdErr("\nProcess died with SIGSEGV (Segmentation fault).\n");
      exit(1);
    } else if (signal == SIGINT) {
      g_pEventProcessor->gotSigINT();
    } else if (signal == SIGTERM or signal == SIGQUIT) {
      g_pEventProcessor->gotSigTERM();
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
}

pEventProcessor::pEventProcessor() : EventProcessor(),
  m_procHandler(new ProcHandler()),
  m_histoman(nullptr)
{
  g_pEventProcessor = this;
}


pEventProcessor::~pEventProcessor()
{
  g_pEventProcessor = nullptr;
  delete m_procHandler;
}

void pEventProcessor::cleanup()
{
  if (!m_procHandler->parallelProcessingUsed() or m_procHandler->isOutputProcess()) {
    m_rbinlist.clear();
    m_rboutlist.clear();
  }
}


void pEventProcessor::gotSigINT()
{
  static int numSigInts = 0;
  if (numSigInts == 0) {
    EventProcessor::writeToStdErr("\nStopping basf2 after all events in buffer are processed. Press Ctrl+C a second time to discard half-processed events.\n");
    //SIGINT is handled independently by input process, so we don't need to do anything special
  } else {
    EventProcessor::writeToStdErr("\nDiscarding pending events for quicker termination... \n");
    //clear ringbuffers
    for (auto rb : m_rbinlist) {
      rb->clear();
    }
    for (auto rb : m_rboutlist) {
      rb->tryClear(); //might deadlock if we're already locked
    }
  }
  numSigInts++;
}

void pEventProcessor::gotSigTERM()
{
  for (auto rb : m_rbinlist) {
    rb->kill();
  }
  for (auto rb : m_rboutlist) {
    rb->kill(); //atomic, so doesn't lock
  }
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


  if (!m_inpathlist.empty()) {
    B2INFO("Input Path " << m_inpathlist[0]->getPathString());
  }
  if (!m_mainpathlist.empty()) {
    B2INFO("Main Path " << m_mainpathlist[0]->getPathString());
  }
  if (!m_outpathlist.empty()) {
    B2INFO("Output Path " << m_outpathlist[0]->getPathString());
  }
  if (m_mainpathlist.empty()) {
    B2WARNING("Cannot run any modules in parallel (no c_ParallelProcessingCertified flag), falling back to single-core mode.");
    EventProcessor::process(spath, maxEvent);
    return;
  }

  setupSignalHandler();

  //inserts Rx/Tx modules into path (sets up IPC structures)
  preparePaths();

  Path mergedPath;
  if (!m_inpathlist.empty())
    mergedPath.addPath(m_inpathlist[0]);
  mergedPath.addPath(m_mainpathlist[0]);
  if (!m_outpathlist.empty())
    mergedPath.addPath(m_outpathlist[0]);

  //init statistics
  m_processStatisticsPtr.registerInDataStore();

  // ensure that we free the IPC resources!
  atexit(cleanupIPC);

  if (!m_processStatisticsPtr)
    m_processStatisticsPtr.create();
  ModulePtrList mergedPathModules = mergedPath.buildModulePathList(); //all modules, including Rx and Tx
  for (ModulePtrList::const_iterator listIter = mergedPathModules.begin(); listIter != mergedPathModules.end(); ++listIter) {
    Module* module = listIter->get();
    m_processStatisticsPtr->initModule(module);
  }

  // 2. Initialization
  ModulePtrList modulelist = spath->buildModulePathList();
  ModulePtrList initGlobally = getModulesWithoutFlag(modulelist, Module::c_InternalSerializer);
  //dump_modules("Initializing globally: ", initGlobally);
  try {
    processInitialize(initGlobally);
  } catch (StoppedBySignalException& e) {
    B2FATAL(e.what());
  }

  //initialization finished, disable handler again
  if (signal(SIGINT, SIG_IGN) == SIG_ERR) {
    B2FATAL("Cannot ignore SIGINT signal handler\n");
  }
  if (signal(SIGTERM, SIG_IGN) == SIG_ERR) {
    B2FATAL("Cannot ignore SIGTERM signal handler\n");
  }
  if (signal(SIGQUIT, SIG_IGN) == SIG_ERR) {
    B2FATAL("Cannot ignore SIGQUIT signal handler\n");
  }

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


  //Path for current process
  PathPtr localPath;

  // 3. Fork input path
  m_procHandler->startInputProcess();
  if (m_procHandler->isInputProcess()) {   // In input process
    localPath = m_inpathlist[0];
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

  if (localPath == nullptr) { //not forked yet -> this process is the output process
    m_procHandler->startOutputProcess();
    if (!m_outpathlist.empty()) {
      localPath = m_outpathlist[0];
      m_master = localPath->getModules().begin()->get(); //set Rx as master
    }

    //ignore some signals for framework (mother) process, so only child processes will handle them
    //once they are finished, the framework process will clean up IPC structures
    if (signal(SIGTERM, signalHandler) == SIG_ERR) {
      B2FATAL("Cannot setup SIGTERM signal handler\n");
    }
    if (signal(SIGQUIT, signalHandler) == SIG_ERR) {
      B2FATAL("Cannot setup SIGQUIT signal handler\n");
    }
    //If our children are killed, we want to know. (especially since this may happen in any order)
    if (signal(SIGCHLD, signalHandler) == SIG_ERR) {
      B2FATAL("Cannot setup SIGCHLD signal handler\n");
    }
    //first time: just print a message, afterwards tries to terminate more quickly
    if (signal(SIGINT, signalHandler) == SIG_ERR) {
      B2FATAL("Cannot set SIGINT signal handler\n");
    }
  }


  if (!m_procHandler->isOutputProcess()) {
    DataStoreStreamer::removeSideEffects();
  }

  if (localPath != nullptr) {
    ModulePtrList localModules = localPath->buildModulePathList();
    ModulePtrList procinitmodules = getModulesWithFlag(localModules, Module::c_InternalSerializer);
    //dump_modules("processInitialize for ", procinitmodules);
    processInitialize(procinitmodules);

    if (m_procHandler->isInputProcess())
      setupSignalHandler();

    try {
      processCore(localPath, localModules, maxEvent);
    } catch (StoppedBySignalException& e) {
      if (e.signal != SIGINT) {
        B2FATAL(e.what());
      }
      //in case of SIGINT, we move on to processTerminate() to shut down saefly
    }
    prependModulesIfNotPresent(&localModules, terminateGlobally);
    processTerminate(localModules);
  }

  B2INFO(m_procHandler->getProcessName() << " process finished.");

  //output process does final cleanup, everything else stops here
  if (!m_procHandler->isOutputProcess()) {
    exit(0);
  }

  //TODO: still needed? might be important for cleaning up after crashes
  m_procHandler->waitForInputProcesses();
  m_procHandler->waitForEventProcesses();
  B2INFO("All processes completed");

  //finished, disable handler again
  if (signal(SIGINT, SIG_IGN) == SIG_ERR) {
    B2FATAL("Cannot ignore SIGINT signal handler\n");
  }

  cleanup();
  B2INFO("Global process: completed");

  //did anything bad happen?
  if (s_PIDofKilledChild != 0) {
    //fatal, so we get appropriate return code
    B2FATAL("Execution stopped, sub-process with PID " << s_PIDofKilledChild << " killed by signal.");
  }

}

void pEventProcessor::analyzePath(const PathPtr& path)
{
  //TODO this should either be properly generalized so it can deal with multiple parallel sections,
  //or one can simply get rid of the different lists for paths...
  PathPtr inpath(new Path);
  PathPtr mainpath(new Path);
  PathPtr outpath(new Path);

  bool createAllPaths = false; //usually we might not need e.g. an output path
  int stage = 0; //0: in, 1: event/main, 2: out
  for (const ModulePtr & module : path->getModules()) {
    //TODO also check conditional path, or have setConditionPath() update module flag...
    const bool hasParallelFlag = module->hasProperties(Module::c_ParallelProcessingCertified);

    if (module->hasProperties(Module::c_TerminateInAllProcesses))
      createAllPaths = true; //ensure there are all kinds of processes

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

  //if main path is empty, createAllPaths doesn't really matter, since we'll fall back to single-core processing
  if (!mainpath->isEmpty())
    m_mainpathlist.push_back(mainpath);
  if (createAllPaths or !inpath->isEmpty())
    m_inpathlist.push_back(inpath);
  if (createAllPaths or !outpath->isEmpty())
    m_outpathlist.push_back(outpath);
}

RingBuffer* pEventProcessor::connectViaRingBuffer(const char* name, PathPtr a, PathPtr& b)
{
  //create ringbuffers and add rx/tx where needed
  const char* inrbname = getenv(name);
  RingBuffer* rbuf;
  if (inrbname == NULL) {
    rbuf = new RingBuffer();
  } else {
    string rbname(inrbname + to_string(0)); //currently at most one input, one output buffer
    rbuf = new RingBuffer(rbname.c_str(), RingBuffer::c_DefaultSize);
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
  for (const ModulePtr & m : modules) {
    if (m->hasProperties(flag))
      tmpModuleList.push_back(m);
  }

  return tmpModuleList;
}
ModulePtrList pEventProcessor::getModulesWithoutFlag(const ModulePtrList& modules, Module::EModulePropFlags flag)
{
  ModulePtrList tmpModuleList;
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

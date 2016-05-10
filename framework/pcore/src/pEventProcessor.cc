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
#include <framework/pcore/RbTuple.h>

#include <framework/core/ModuleManager.h>
#include <framework/core/Environment.h>
#include <framework/logging/LogSystem.h>

#include <TROOT.h>

#include <signal.h>
#include <sys/wait.h>
#include <cstdlib>


using namespace std;
using namespace Belle2;

namespace {
  static int gSignalReceived = 0;

  static pEventProcessor* g_pEventProcessor = NULL;

  void cleanupIPC()
  {
    if (g_pEventProcessor)
      g_pEventProcessor->cleanup();
  }
  void cleanupAndStop(int sig)
  {
    cleanupIPC();

    //uninstall current handler and call default one.
    signal(sig, SIG_DFL);
    raise(sig);
  }

  static void parentSignalHandler(int signal)
  {
    //signal handlers are called asynchronously, making many standard functions (including output) dangerous
    //write() is, however, safe, so we'll use that to write to stderr.
    if (signal == SIGSEGV) {
      EventProcessor::writeToStdErr("\nProcess died with SIGSEGV (Segmentation fault).\n");
      exit(1);
    } else if (signal == SIGINT) {
      g_pEventProcessor->gotSigINT();
    } else if (signal == SIGTERM or signal == SIGQUIT) {
      g_pEventProcessor->killRingBuffers();
    }
    if (gSignalReceived == 0)
      gSignalReceived = signal;
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
  EventProcessor::writeToStdErr("\nStopping basf2...\n");
  killRingBuffers();
}

void pEventProcessor::killRingBuffers()
{
  for (auto rb : m_rbinlist) {
    rb->kill();
  }
  //these might be locked by _this_ process, so we cannot escape
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

  if (numProcesses == 0)
    B2FATAL("pEventProcessor::process() called for serial processing! Most likely a bug in Framework.");

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

  installMainSignalHandlers(cleanupAndStop);

  //inserts Rx/Tx modules into path (sets up IPC structures)
  preparePaths();

  // ensure that we free the IPC resources!
  atexit(cleanupIPC);


  //init statistics
  {
    m_processStatisticsPtr.registerInDataStore();
    if (!m_processStatisticsPtr)
      m_processStatisticsPtr.create();
    Path mergedPath;
    if (!m_inpathlist.empty())
      mergedPath.addPath(m_inpathlist[0]);
    mergedPath.addPath(m_mainpathlist[0]);
    if (!m_outpathlist.empty())
      mergedPath.addPath(m_outpathlist[0]);
    for (ModulePtr module : mergedPath.buildModulePathList())
      m_processStatisticsPtr->initModule(module.get());
  }

  // 2. Initialization
  ModulePtrList modulelist = spath->buildModulePathList();
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


  //Path for current process
  PathPtr localPath;

  // 3. Fork input path
  m_procHandler->startInputProcess();
  if (m_procHandler->isInputProcess()) {   // In input process
    localPath = m_inpathlist[0];
  } else {
    // This is not the input path, clean up datastore to not contain the first event
    DataStore::Instance().invalidateData(DataStore::c_Event);
  }

  if (localPath == nullptr) { //not forked yet
    // 5. Fork out worker path (parallel section)
    m_procHandler->startWorkerProcesses(numProcesses);
    if (m_procHandler->isWorkerProcess()) {
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

    installMainSignalHandlers(parentSignalHandler);
  }


  if (!m_procHandler->isOutputProcess()) {
    DataStoreStreamer::removeSideEffects();
  }

  bool gotSigINT = false;
  if (localPath != nullptr) {
    ModulePtrList localModules = localPath->buildModulePathList();
    ModulePtrList procinitmodules = getModulesWithFlag(localModules, Module::c_InternalSerializer);
    //dump_modules("processInitialize for ", procinitmodules);
    processInitialize(procinitmodules);

    //input: handle signals normally, will slowly cascade down
    if (m_procHandler->isInputProcess())
      installMainSignalHandlers();
    //workers will have to ignore the signals, there's no good way to do this safely
    if (m_procHandler->isWorkerProcess())
      installMainSignalHandlers(SIG_IGN);

    try {
      processCore(localPath, localModules, maxEvent, m_procHandler->isInputProcess());
    } catch (StoppedBySignalException& e) {
      if (e.signal != SIGINT) {
        B2FATAL(e.what());
      }
      //in case of SIGINT, we move on to processTerminate() to shut down saefly
      gotSigINT = true;
    }
    prependModulesIfNotPresent(&localModules, terminateGlobally);
    processTerminate(localModules);
  }

  B2INFO(m_procHandler->getProcessName() << " process finished.");

  //output process does final cleanup, everything else stops here
  if (!m_procHandler->isOutputProcess()) {
    if (gotSigINT) {
      B2FATAL("Processing aborted via SIGINT.");
    } else {
      exit(0);
    }
  }

  m_procHandler->waitForAllProcesses();
  B2INFO("All processes completed");

  //finished, disable handler again
  installSignalHandler(SIGINT, SIG_IGN);

  cleanup();
  B2INFO("Global process: completed");

  if (m_histoman) {
    B2INFO("HistoManager:: adding histogram files");
    RbTupleManager::Instance().hadd();
  }

  //did anything bad happen?
  if (gSignalReceived) {
    B2FATAL("Processing aborted via signal " << gSignalReceived <<
            ", terminating. Output files have been closed safely and should be readable.");
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
  for (const ModulePtr& module : path->getModules()) {
    bool hasParallelFlag = module->hasProperties(Module::c_ParallelProcessingCertified);
    //entire conditional path must also be compatible
    if (hasParallelFlag and module->hasCondition()) {
      if (!ModuleManager::allModulesHaveFlag(module->getConditionPath()->getModules(), Module::c_ParallelProcessingCertified))
        hasParallelFlag = false;
    }

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
  for (const ModulePtr& m : modules) {
    if (m->hasProperties(flag))
      tmpModuleList.push_back(m);
  }

  return tmpModuleList;
}
ModulePtrList pEventProcessor::getModulesWithoutFlag(const ModulePtrList& modules, Module::EModulePropFlags flag)
{
  ModulePtrList tmpModuleList;
  for (const ModulePtr& m : modules) {
    if (!m->hasProperties(flag))
      tmpModuleList.push_back(m);
  }
  return tmpModuleList;
}

void pEventProcessor::prependModulesIfNotPresent(ModulePtrList* modules, const ModulePtrList& prependModules)
{
  for (const ModulePtr& m : prependModules) {
    if (std::find(modules->begin(), modules->end(), m) == modules->end()) { //not present
      modules->push_front(m);
    }
  }
}

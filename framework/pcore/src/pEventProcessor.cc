/*
 * pEventProcessor.cc
 *
 *  Created on: Oct 26, 2010
 *      Author: R.Itoh, IPNS, KEK
 *  Updated : Jul. 25, 2011
 */

#include <framework/pcore/pEventProcessor.h>
#include <framework/pcore/zeromq/RandomNameGenerator.h>
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

#include <chrono>
#include <thread>

#include <signal.h>
#include <fstream>

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
    if (signal == SIGINT) {
      g_pEventProcessor->gotSigINT();
    } else if (signal == SIGTERM or signal == SIGQUIT) {
      g_pEventProcessor->killRingBuffers();
    }
    if (gSignalReceived == 0)
      gSignalReceived = signal;
  }

}

pEventProcessor::pEventProcessor() : EventProcessor(),
  m_histoman(nullptr)
{
  g_pEventProcessor = this;
}


pEventProcessor::~pEventProcessor()
{
  cleanup();
  g_pEventProcessor = nullptr;
}

void pEventProcessor::cleanup()
{
  if (!m_procHandler->parallelProcessingUsed() or m_procHandler->isMonitoringProcess()) {
    std::remove(g_pEventProcessor->m_inputSocketName.c_str());
    std::remove(g_pEventProcessor->m_outputSocketName.c_str());
  }
}


void pEventProcessor::gotSigINT()
{
  EventProcessor::writeToStdErr("\nStopping basf2...\n");
  killRingBuffers();
}

void pEventProcessor::killRingBuffers()
{
  //m_rbin->kill();
  //these might be locked by _this_ process, so we cannot escape
  //m_rbout->kill(); //atomic, so doesn't lock
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
  unsigned int numEventsArgument = Environment::Instance().getNumberEventsOverride();
  if ((numEventsArgument > 0) && ((maxEvent == 0) || (maxEvent > numEventsArgument))) {
    maxEvent = numEventsArgument;
  }

  if (numProcesses == 0)
    B2FATAL("pEventProcessor::process() called for serial processing! Most likely a bug in Framework.");

  // 1. Analyze start path and split into parallel paths
  analyzePath(spath);

  if (not m_mainPath) {
    B2WARNING("Cannot run any modules in parallel (no c_ParallelProcessingCertified flag), falling back to single-core mode.");
    EventProcessor::process(spath, maxEvent);
    return;
  }

  // Choose names for the input and output socket
  m_inputSocketName = random_socket_name(m_socketProtocol == "tcp");
  m_outputSocketName = random_socket_name(m_socketProtocol == "tcp");

  installMainSignalHandlers(cleanupAndStop);

  //inserts Rx/Tx modules into path (sets up IPC structures)
  preparePaths();

  if (m_inputPath) {
    B2INFO("Input Path " << m_inputPath->getPathString());
  }
  if (m_mainPath) {
    B2INFO("Main Path " << m_mainPath->getPathString());
  }
  if (m_outputPath) {
    B2INFO("Output Path " << m_outputPath->getPathString());
  }

  // ensure that we free the IPC resources!
  atexit(cleanupIPC);

  //init statistics
  m_processStatisticsPtr.registerInDataStore();
  if (!m_processStatisticsPtr)
    m_processStatisticsPtr.create();
  Path mergedPath;
  if (m_inputPath)
    mergedPath.addPath(m_inputPath);
  mergedPath.addPath(m_mainPath);
  if (m_outputPath)
    mergedPath.addPath(m_outputPath);
  for (ModulePtr module : mergedPath.buildModulePathList())
    m_processStatisticsPtr->initModule(module.get());

  // 2. Initialization
  ModulePtrList modulelist = mergedPath.buildModulePathList();
  processInitialize(modulelist, false);

  ModulePtrList terminateGlobally = getModulesWithFlag(modulelist, Module::c_TerminateInAllProcesses);

  //Don't start processing in case of no master module
  if (!m_master) {
    B2ERROR("There is no module that provides event and run numbers. You must either add the EventInfoSetter module to your path, or, if using an input module, read EventMetaData objects from file.");
  }

  // Check if errors appeared. If yes, don't start the event processing.
  int numLogError = LogSystem::Instance().getMessageCounter(LogConfig::c_Error);
  if (numLogError != 0) {
    B2FATAL(numLogError << " ERROR(S) occurred! The processing of events will not be started.");
  }

  //disable ROOT's management of TFiles
  clearFileList();

  //install new signal handlers before forking
  installMainSignalHandlers(parentSignalHandler);

  //Path for current process
  PathPtr localPath;

  m_procHandler.reset(new ProcHandler(numProcesses));

  // 3. Fork input path
  m_procHandler->startInputProcess();
  if (m_procHandler->isInputProcess()) {   // In input process
    if (m_inputPath and not m_inputPath->isEmpty()) {
      localPath = m_inputPath;
    }
  } else {
    // This is not the input path, clean up datastore to not contain the first event
    DataStore::Instance().invalidateData(DataStore::c_Event);

    // 5. Fork out worker path (parallel section)
    m_procHandler->startWorkerProcesses();
    if (m_procHandler->isWorkerProcess()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      localPath = m_mainPath;
      if (m_inputPath and not m_inputPath->isEmpty()) {
        m_master = localPath->getModules().begin()->get(); //set Rx as master
      }
    } else {
      m_procHandler->startOutputProcess();
      if (m_procHandler->isOutputProcess()) {
        if (m_outputPath and not m_outputPath->isEmpty()) {
          localPath = m_outputPath;
          m_master = localPath->getModules().begin()->get(); //set Rx as master
        }
      } else {
        // Still not forked: this is the monitor process
        m_procHandler->setAsMonitoringProcess();
      }
    }
  }

  if (m_procHandler->isMonitoringProcess())
    installMainSignalHandlers(SIG_IGN);
  else
    installMainSignalHandlers();


  B2RESULT("Running as " << m_procHandler->getProcessName());

  // This is very all processes and up:
  if (not m_procHandler->isOutputProcess() and not m_procHandler->isMonitoringProcess()) {
    DataStoreStreamer::removeSideEffects();
  }

  bool gotSigINT = false;
  if (localPath != nullptr) {
    ModulePtrList localModules = localPath->buildModulePathList();
    try {
      processCore(localPath, localModules, maxEvent, false);
    } catch (StoppedBySignalException& e) {
      if (e.signal != SIGINT) {
        B2FATAL(e.what());
      }
      //in case of SIGINT, we move on to processTerminate() to shut down safely
      gotSigINT = true;
    }
    prependModulesIfNotPresent(&localModules, terminateGlobally);
    processTerminate(localModules);
  }

  // all processes except monitor stop here
  if (!m_procHandler->isMonitoringProcess()) {
    B2INFO(m_procHandler->getProcessName() << " process finished.");
    if (gotSigINT) {
      installSignalHandler(SIGINT, SIG_DFL);
      raise(SIGINT);
    } else {
      exit(0);
    }
  }

  //monitor process: do final cleanup
  B2INFO("Waiting for all processes to finish.");
  m_procHandler->waitForAllProcesses();
  B2INFO("All processes completed.");

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
    B2ERROR("Processing aborted via signal " << gSignalReceived <<
            ", terminating. Output files have been closed safely and should be readable.");
    installSignalHandler(gSignalReceived, SIG_DFL);
    raise(gSignalReceived);
  }

}

void pEventProcessor::analyzePath(const PathPtr& path)
{
  //modules that can be parallelised, but should not go into a parallel section by themselves
  std::set<std::string> uselessParallelModules({"HistoManager", "Gearbox", "Geometry"});

  PathPtr inpath(new Path);
  PathPtr mainpath(new Path);
  PathPtr outpath(new Path);

  int stage = 0; //0: in, 1: event/main, 2: out
  for (const ModulePtr& module : path->getModules()) {
    bool hasParallelFlag = module->hasProperties(Module::c_ParallelProcessingCertified);
    //entire conditional path must also be compatible
    if (hasParallelFlag and module->hasCondition()) {
      for (const auto& conditionPath : module->getAllConditionPaths()) {
        if (!ModuleManager::allModulesHaveFlag(conditionPath->getModules(), Module::c_ParallelProcessingCertified)) {
          hasParallelFlag = false;
        }
      }
    }

    //update stage?
    if ((stage == 0 and hasParallelFlag) or (stage == 1 and !hasParallelFlag)) {
      stage++;

      if (stage == 2) {
        bool path_is_useful = false;
        for (auto parallelModule : mainpath->getModules()) {
          if (uselessParallelModules.count(parallelModule->getType()) == 0) {
            path_is_useful = true;
            break;
          }
        }
        if (not path_is_useful) {
          //merge mainpath back into input path
          inpath->addPath(mainpath);
          mainpath.reset(new Path);
          //and search for further parallel sections
          stage = 0;
        }
      }
    }

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

  bool createAllPaths = false; //usually we might not need e.g. an output path
  for (const ModulePtr& module : path->getModules()) {
    if (module->hasProperties(Module::c_TerminateInAllProcesses)) {
      createAllPaths = true; //ensure there are all kinds of processes
    }
  }

  //if main path is empty, createAllPaths doesn't really matter, since we'll fall back to single-core processing
  if (!mainpath->isEmpty())
    m_mainPath = mainpath;
  if (createAllPaths or !inpath->isEmpty())
    m_inputPath = inpath;
  if (createAllPaths or !outpath->isEmpty()) {
    m_outputPath = outpath;
  }
}

void pEventProcessor::appendModule(PathPtr& path, ModulePtr module)
{
  path->addModule(module);
}

void pEventProcessor::prependModule(PathPtr& path, ModulePtr module)
{
  PathPtr newPath(new Path());
  newPath->addModule(module);
  newPath->addPath(path);
  path.swap(newPath);
}

void pEventProcessor::preparePaths()
{
  if (m_histoman) {
    m_histoman->initialize();
  }
  if (not m_mainPath or m_mainPath->isEmpty())
    return; //we'll fall back to single-core

  ModuleManager& moduleManager = ModuleManager::Instance();


  if (m_inputPath and not m_inputPath->isEmpty()) {
    if (m_inputPath->getModules().size() == 1 and m_inputPath->getModules().front()->getName() == "SeqRootInput") {
      std::string inputFileName = m_inputPath->getModules().front()->getParam<std::string>("inputFileName").getValue();

      m_inputPath.reset(new Path());
      ModulePtr zeroMQTxSeqRootInputModule = moduleManager.registerModule("ZeroMQTxSeqRootInput");
      zeroMQTxSeqRootInputModule->getParam<std::string>("inputFileName").setValue(inputFileName);
      zeroMQTxSeqRootInputModule->getParam<std::string>("socketName").setValue(
        m_socketProtocol + "://" + m_inputSocketName);
      m_inputPath->addModule(zeroMQTxSeqRootInputModule);
    } else {
      ModulePtr zeroMQTxInputModule = moduleManager.registerModule("ZeroMQTxInput");
      zeroMQTxInputModule->getParam<std::string>("socketName").setValue(
        m_socketProtocol + "://" + m_inputSocketName);
      m_inputPath->addModule(zeroMQTxInputModule);
    }

    ModulePtr zeroMQRxInputModule = moduleManager.registerModule("ZeroMQRxInput");
    zeroMQRxInputModule->getParam<std::string>("socketName").setValue(
      m_socketProtocol + "://" + m_inputSocketName);
    prependModule(m_mainPath, zeroMQRxInputModule);
  }
  if (m_outputPath and not m_outputPath->isEmpty()) {
    ModulePtr zeroMQTxOutputModule = moduleManager.registerModule("ZeroMQTxOutput");
    zeroMQTxOutputModule->getParam<std::string>("socketName").setValue(
      m_socketProtocol + "://" + m_outputSocketName);
    appendModule(m_mainPath, zeroMQTxOutputModule);

    if (m_outputPath->getModules().size() == 1 and m_outputPath->getModules().front()->getName() == "SeqRootOutput") {
      std::string outputFileName = m_outputPath->getModules().front()->getParam<std::string>("outputFileName").getValue();

      m_outputPath.reset(new Path());
      ModulePtr zeroMQRxSeqRootOutputModule = moduleManager.registerModule("ZeroMQRxSeqRootOutput");
      zeroMQRxSeqRootOutputModule->getParam<std::string>("outputFileName").setValue(outputFileName);
      zeroMQRxSeqRootOutputModule->getParam<std::string>("socketName").setValue(
        m_socketProtocol + "://" + m_outputSocketName);
      m_outputPath->addModule(zeroMQRxSeqRootOutputModule);
    } else {
      ModulePtr zeroMQRxOutputModule = moduleManager.registerModule("ZeroMQRxOutput");
      zeroMQRxOutputModule->getParam<std::string>("socketName").setValue(
        m_socketProtocol + "://" + m_outputSocketName);
      prependModule(m_outputPath, zeroMQRxOutputModule);
    }
  }
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

//
// Created by abaur on 02.05.18.
//

#include <framework/pcore/ProcHelper.h>
#include <framework/pcore/zmq/processModules/ZMQDefinitions.h>
#include <framework/pcore/zmq/processModules/ZMQHelper.h>

#include <framework/pcore/pEventProcessor.h>
#include <framework/pcore/zmq/processModules/RandomNameGenerator.h>
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



  void cleanupAndStop(int sig)
  {
    if (g_pEventProcessor)
      g_pEventProcessor->cleanup();

    //uninstall current handler and call default one.
    signal(sig, SIG_DFL);
    raise(sig);
  }

// --------------------------- Init / Monitor Process signal handler -------------------------------------------

  static void parentSignalHandler(int signal)
  {
    //signal handlers are called asynchronously, making many standard functions (including output) dangerous
    if (signal == SIGINT) {
      EventProcessor::writeToStdErr("\nStopping basf2...\n");
      g_pEventProcessor->cleanup();
      //g_pEventProcessor->gotSigINT();
    } else if (signal == SIGTERM or signal == SIGQUIT) {

    }
    if (gSignalReceived == 0)
      gSignalReceived = signal;
  }
}
// ----------------------------------------------------------------------------------------------------------------



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
  if (!m_procHandler->parallelProcessingUsed() or m_procHandler->isProcess(ProcType::c_Monitor)) {
    //TODO: enter here the PCB stuff
    if (m_multicastOnline)
      m_procHandler->stopEvtProc();
    // interrupts the proxy and kills all left processes, timeout to hard kill
    m_procHandler->killAllChildProc(5);
  }
}


void pEventProcessor::gotSigINT()
{
  EventProcessor::writeToStdErr("\nStopping basf2...\n");
  killRingBuffers();
  cleanup();
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
  if (spath->getModules().size() == 0) return; // here is nothing to do for us

  const int numProcesses = Environment::Instance().getNumberProcesses();

  //Check whether the number of events was set via command line argument
  unsigned int numEventsArgument = Environment::Instance().getNumberEventsOverride();
  if ((numEventsArgument > 0) && ((maxEvent == 0) || (maxEvent > numEventsArgument))) {
    maxEvent = numEventsArgument;
  }

  if (numProcesses == 0)
    B2FATAL("pEventProcessor::process() called for serial processing! Most likely a bug in Framework.");

  // ====================================================
  // 1. Analyze start path and split into parallel paths
  // ====================================================
  B2DEBUG(100, "Analize path...");
  analyzePath(spath);

  if (not m_mainPath) {
    B2WARNING("Cannot run any modules in parallel (no c_ParallelProcessingCertified flag), falling back to single-core mode.");
    EventProcessor::process(spath, maxEvent);
    return;
  }

  // Choose names for the input and output socket
  m_inputSocketName = random_socket_name(m_socketProtocol == "tcp");
  m_outputSocketName = random_socket_name(m_socketProtocol == "tcp");
  // Name for the socket of the proxy for PCB
  m_xpubProxySocketName = random_socket_name(m_socketProtocol == "tcp");
  m_xsubProxySocketName = random_socket_name(m_socketProtocol == "tcp");


  std::string xpubSocketAddr = ZMQHelper::getSocketAddr(m_xpubProxySocketName, m_socketProtocol);
  std::string xsubSocketAddr = ZMQHelper::getSocketAddr(m_xsubProxySocketName, m_socketProtocol);

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

  // =======================
  // 2. Initialization
  // =======================

  // function called for signals: SIGINT, SIGTERM, SIGQUIT
  // TODO: do we need installMainSignalHandlers here?
  installMainSignalHandlers(cleanupAndStop);
  // ensure that we free the IPC resources when exit!


  //init statistics
  m_processStatisticsPtr.registerInDataStore();
  if (!m_processStatisticsPtr)
    m_processStatisticsPtr.create();

  //add modules to statistics
  Path mergedPath;
  if (m_inputPath)
    mergedPath.addPath(m_inputPath);
  mergedPath.addPath(m_mainPath);
  if (m_outputPath)
    mergedPath.addPath(m_outputPath);

  for (ModulePtr module : mergedPath.buildModulePathList())
    m_processStatisticsPtr->initModule(module.get());

  ModulePtrList modulelist = mergedPath.buildModulePathList();
  // from now datastore available
  processInitialize(modulelist, false);

  ModulePtrList terminateGlobally = ProcHelper::getModulesWithFlag(modulelist, Module::c_TerminateInAllProcesses);

  //Don't start processing in case of no master module
  if (!m_master) {
    B2ERROR("There is no module that provides event and run numbers. You must either add the EventInfoSetter module to your path, or, if using an input module, read EventMetaData objects from file.");
  }

  // Check if errors appeared. If yes, don't start the event processing.
  int numLogError = LogSystem::Instance().getMessageCounter(LogConfig::c_Error);
  if (numLogError != 0) {
    B2FATAL(numLogError << " ERROR(S) occurred! The processing of events will not be started.");
  }

  // the handler for forking and handling all processes
  m_procHandler.reset(new ProcHandler(numProcesses));

  //disable ROOT's management of TFiles
  clearFileList();

  ///install new signal handlers before forking, function called for signals: SIGINT, SIGTERM, SIGQUIT
  installMainSignalHandlers(parentSignalHandler);
  //Path for current process
  PathPtr localPath;

  // =====================
  // 3. Fork proxy process
  // =====================
  m_procHandler->startProxyProcess(xpubSocketAddr, xsubSocketAddr);
  if (m_procHandler->isProcess(ProcType::c_Proxy)) {
    // proxy is blocking
  } else {
    m_procHandler->initPCBMulticast(xpubSocketAddr, xsubSocketAddr); // the multicast for the monitoring process
    m_procHandler->subscribePCBMulticast(c_MessageTypes::c_helloMessage);
    B2DEBUG(100, "Multicast for Init Process was set up");

    // =====================
    // 4. Fork input path
    // =====================
    m_procHandler->startInputProcess();
    if (m_procHandler->isProcess(ProcType::c_Input)) {   // input process gets the path of input modules
      if (m_inputPath and not m_inputPath->isEmpty()) {
        localPath = m_inputPath;
      }
    } else {
      // This is not the input process, clean up datastore to not contain the first event
      DataStore::Instance().invalidateData(DataStore::c_Event);

      // =================================
      // 5. Fork out output path
      // =================================
      m_procHandler->startOutputProcess();
      if (m_procHandler->isProcess(ProcType::c_Output)) {
        if (m_outputPath and not m_outputPath->isEmpty()) {
          localPath = m_outputPath;
          m_master = localPath->getModules().begin()->get(); //set Rx as master
        }
      } else {
        if (m_procHandler->waitForStartEvtProc()) {
          B2INFO("Input and Output online, start event processing...");

          // ===========================================
          // 6. Fork out worker path (parallel section)
          // ===========================================
          m_procHandler->startWorkerProcesses();
          if (m_procHandler->isProcess(ProcType::c_Worker)) {
            localPath = m_mainPath;
            if (m_inputPath and not m_inputPath->isEmpty()) {
              m_master = localPath->getModules().begin()->get(); //set Rx as master
            }
          } else {
            // still in parent process: the init process becomes now the monitor process
            m_procHandler->setAsMonitoringProcess();
          }
        } else {
          // TODO : on fail need to kill all child processes ?
          //m_procHandler->killAllChildProc();
          B2FATAL("Not able to start event processing... aborting");
        }
      }
    }
  }

  if (m_procHandler->isProcess(ProcType::c_Monitor)) {
    // Monitoring process ignores SIGINT, SIGTERM, SIGQUIT

  } else
    installMainSignalHandlers(); // Main signals have no effect, still do this in prochandler::startproc

  B2RESULT("Running as " << m_procHandler->getProcessName());

  // This is very all processes and up:
  if (not m_procHandler->isProcess(ProcType::c_Output) and not m_procHandler->isProcess(ProcType::c_Monitor)) {
    DataStoreStreamer::removeSideEffects();
  }

  bool gotSigINT = false;

  if (localPath != nullptr) { // if not monitoring process then process the module paths
    ModulePtrList localModules = localPath->buildModulePathList();
    try {
      // ======================================
      // 7. here all the modules are processed
      // ======================================
      processCore(localPath, localModules, maxEvent, false);
      B2INFO("After process core");
    } catch (StoppedBySignalException& e) {
      if (e.signal != SIGINT) {
        B2FATAL(e.what());
      }
      B2INFO("Stopped by exception");
      //in case of SIGINT, we move on to processTerminate() to shut down safely
      gotSigINT = true;
    }
    // TODO: replace with terminateProcess(&localModules, terminateGlobally); really needed?

    B2DEBUG(100, "terminate process...");
    ProcHelper::prependModulesIfNotPresent(&localModules, terminateGlobally);
    // process the safe shutdown
    processTerminate(localModules);
  }

  // ============================================
  // 8. all processes stop here except monitor
  // ============================================
  if (not m_procHandler->isProcess(ProcType::c_Monitor)) {
    B2INFO(m_procHandler->getProcessName() << " process finished.");
    exit(0);
  }

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
    //if modules have parallal flag -> stage = 1 , event/main
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


void pEventProcessor::preparePaths()
{
  if (m_histoman) {
    m_histoman->initialize();
  }
  if (not m_mainPath or m_mainPath->isEmpty())
    return; //we'll fall back to single-core

  ModuleManager& moduleManager = ModuleManager::Instance();

  std::string xpubSocketAddr = ZMQHelper::getSocketAddr(m_xpubProxySocketName, m_socketProtocol);
  std::string xsubSocketAddr = ZMQHelper::getSocketAddr(m_xsubProxySocketName, m_socketProtocol);

  // ========================
  // setup input -> worker modules
  // ========================
  if (m_inputPath and not m_inputPath->isEmpty()) {
    // SeqRoot input
    if (m_inputPath->getModules().size() == 1 and m_inputPath->getModules().front()->getName() == "SeqRootInput") {
      std::string inputFileName = m_inputPath->getModules().front()->getParam<std::string>("inputFileName").getValue();

      m_inputPath.reset(new Path());
      ModulePtr zmqTxSeqRootInputModule = moduleManager.registerModule("ZMQTxSeqRootInput");
      zmqTxSeqRootInputModule->getParam<std::string>("inputFileName").setValue(inputFileName);
      zmqTxSeqRootInputModule->getParam<std::string>("socketName").setValue(
        m_socketProtocol + "://" + m_inputSocketName);
      zmqTxSeqRootInputModule->getParam<std::string>("xpubProxySocketName").setValue(
        xpubSocketAddr);
      zmqTxSeqRootInputModule->getParam<std::string>("xsubProxySocketName").setValue(
        xsubSocketAddr);
      m_inputPath->addModule(zmqTxSeqRootInputModule);
    }
    // Normal Input
    else {
      ModulePtr zmqTxInputModule = moduleManager.registerModule("ZMQTxInput");
      zmqTxInputModule->getParam<std::string>("socketName").setValue(
        m_socketProtocol + "://" + m_inputSocketName);;
      zmqTxInputModule->getParam<std::string>("xpubProxySocketName").setValue(
        xpubSocketAddr);
      zmqTxInputModule->getParam<std::string>("xsubProxySocketName").setValue(
        xsubSocketAddr);
      m_inputPath->addModule(zmqTxInputModule);
    }
    B2DEBUG(100, "Setup Path for inputmoduels");
    // Receive worker
    ModulePtr zmqRxWorkerModule = moduleManager.registerModule("ZMQRxWorker");
    zmqRxWorkerModule->getParam<std::string>("socketName").setValue(
      m_socketProtocol + "://" + m_inputSocketName);
    zmqRxWorkerModule->getParam<std::string>("xpubProxySocketName").setValue(
      xpubSocketAddr);
    zmqRxWorkerModule->getParam<std::string>("xsubProxySocketName").setValue(
      xsubSocketAddr);
    ProcHelper::prependModule(m_mainPath, zmqRxWorkerModule); //set zmqRXWorker before mainpath
  }

  // ========================
  // setup worker -> output modules
  // ========================
  if (m_outputPath and not m_outputPath->isEmpty()) {
    // Transmit worker
    ModulePtr zmqTxWorkerModule = moduleManager.registerModule("ZMQTxWorker");
    zmqTxWorkerModule->getParam<std::string>("socketName").setValue(
      m_socketProtocol + "://" + m_outputSocketName);
    zmqTxWorkerModule->getParam<std::string>("xpubProxySocketName").setValue(
      xpubSocketAddr);
    zmqTxWorkerModule->getParam<std::string>("xsubProxySocketName").setValue(
      xsubSocketAddr);
    ProcHelper::appendModule(m_mainPath, zmqTxWorkerModule);

    B2DEBUG(100, "Setup Path for workermoduels");

    // SeqRoot output
    if (m_outputPath->getModules().size() == 1 and m_outputPath->getModules().front()->getName() == "SeqRootOutput") {
      std::string outputFileName = m_outputPath->getModules().front()->getParam<std::string>("outputFileName").getValue();

      m_outputPath.reset(new Path());
      ModulePtr zmqRxSeqRootOutputModule = moduleManager.registerModule("ZMQRxSeqRootOutput");
      zmqRxSeqRootOutputModule->getParam<std::string>("outputFileName").setValue(outputFileName);
      zmqRxSeqRootOutputModule->getParam<std::string>("socketName").setValue(
        m_socketProtocol + "://" + m_outputSocketName);
      zmqRxSeqRootOutputModule->getParam<std::string>("xpubProxySocketName").setValue(
        xpubSocketAddr);
      zmqRxSeqRootOutputModule->getParam<std::string>("xsubProxySocketName").setValue(
        xsubSocketAddr);
      m_outputPath->addModule(zmqRxSeqRootOutputModule);
    }
    // Normal output
    else {
      ModulePtr zmqRxOutputModule = moduleManager.registerModule("ZMQRxOutput");
      zmqRxOutputModule->getParam<std::string>("socketName").setValue(
        m_socketProtocol + "://" + m_outputSocketName);
      zmqRxOutputModule->getParam<std::string>("xpubProxySocketName").setValue(
        xpubSocketAddr);
      zmqRxOutputModule->getParam<std::string>("xsubProxySocketName").setValue(
        xsubSocketAddr);
      ProcHelper::prependModule(m_outputPath, zmqRxOutputModule);
    }
    B2DEBUG(100, "Setup Path for outputmoduels");
  }
}



void pEventProcessor::terminateProcesses(ModulePtrList* modules, const ModulePtrList& prependModules)
{
  int timeout = 5;
  if (m_procHandler->isProcess(ProcType::c_Init)) {
    m_procHandler->killAllChildProc();
  }
  if (m_procHandler->isProcess(ProcType::c_Monitor)) {
    m_procHandler->stopEvtProc(); // sends terminate message across multicast
    sleep(timeout);

    ProcHelper::prependModulesIfNotPresent(modules, prependModules);
    // process the safe shutdown
    processTerminate(*modules);

    m_procHandler->killAllChildProc(); // shutdown Proxy and kill what in the pid list
  }

}





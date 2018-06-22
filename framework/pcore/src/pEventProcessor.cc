//
// Created by abaur on 02.05.18.
//

#include <framework/pcore/ProcHelper.h>
#include <framework/pcore/zmq/processModules/ZMQDefinitions.h>
#include <framework/pcore/zmq/processModules/ZMQHelper.h>
#include <framework/pcore/PathUtils.h>

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
      EventProcessor::writeToStdErr("\nStopping basf2 gracefully...\n");
      g_pEventProcessor->cleanup();
      //g_pEventProcessor->gotSigINT();
    } else if (signal == SIGTERM or signal == SIGQUIT) {

    }
    if (gSignalReceived == 0)
      gSignalReceived = signal;
  }
}
// ----------------------------------------------------------------------------------------------------------------



pEventProcessor::pEventProcessor(const std::string& socketAddress) : EventProcessor(),
  m_socketAddress(socketAddress)
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
  cleanup();
}

void pEventProcessor::clearFileList()
{
  //B2WARNING("list of files: " << gROOT->GetListOfFiles()->GetEntries());
  //clear list, but don't actually delete the objects
  gROOT->GetListOfFiles()->Clear("nodelete");
}

void pEventProcessor::process(PathPtr path, long maxEvent)
{
  if (path->isEmpty()) {
    // here is nothing to do for us
    return;
  }

  //Check whether the number of events was set via command line argument
  const int numProcesses = Environment::Instance().getNumberProcesses();
  if (numProcesses == 0) {
    B2FATAL("pEventProcessor::process() called for serial processing! Most likely a bug in Framework.");
  }

  PathPtr inputPath, mainPath, outputPath;
  std::tie(inputPath, mainPath, outputPath) = PathUtils::splitPath(path);
  ModulePtr m_histoman = PathUtils::getHistogramManager(inputPath, mainPath, outputPath);

  if (not mainPath or mainPath->isEmpty()) {
    B2WARNING("Cannot run any modules in parallel (no c_ParallelProcessingCertified flag), falling back to single-core mode.");
    EventProcessor::process(path, maxEvent);
    return;
  }

  // inserts Rx/Tx modules into path (sets up IPC structures)
  PathUtils::preparePaths(inputPath, mainPath, outputPath, m_socketAddress);

  // =======================
  // 2. Initialization
  // =======================

  // function called for signals: SIGINT, SIGTERM, SIGQUIT
  // TODO: do we need installMainSignalHandlers here?
  installMainSignalHandlers(cleanupAndStop);
  // ensure that we free the IPC resources when exit!

  if (m_histoman) {
    m_histoman->initialize();
  }

  //init statistics
  m_processStatisticsPtr.registerInDataStore();
  if (!m_processStatisticsPtr)
    m_processStatisticsPtr.create();

  //add modules to statistics
  Path mergedPath;
  if (inputPath)
    mergedPath.addPath(inputPath);
  mergedPath.addPath(mainPath);
  if (outputPath)
    mergedPath.addPath(outputPath);

  for (ModulePtr module : mergedPath.buildModulePathList())
    m_processStatisticsPtr->initModule(module.get());

  ModulePtrList modulelist = mergedPath.buildModulePathList();
  // from now datastore available
  processInitialize(modulelist, true);

  ModulePtrList terminateGlobally = PathUtils::getModulesWithFlag(modulelist, Module::c_TerminateInAllProcesses);

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
  const auto pubSocketAddress(ZMQHelper::getSocketAddress(m_socketAddress, ZMQAddressType::c_pub));
  const auto subSocketAddress(ZMQHelper::getSocketAddress(m_socketAddress, ZMQAddressType::c_sub));

  m_procHandler->startProxyProcess(pubSocketAddress, subSocketAddress);
  if (m_procHandler->isProcess(ProcType::c_Proxy)) {
    // proxy is blocking
  } else {

    m_procHandler->initPCBMulticast(pubSocketAddress, subSocketAddress); // the multicast for the monitoring process
    m_procHandler->subscribePCBMulticast(c_MessageTypes::c_helloMessage);
    m_procHandler->subscribePCBMulticast(c_MessageTypes::c_deathMessage); // worker run in process timeout
    m_procHandler->subscribePCBMulticast(c_MessageTypes::c_terminateMessage);  // input is in terminate mode -> no restart of worker

    B2DEBUG(100, "Multicast for Init Process was set up");

    // =====================
    // 4. Fork input path
    // =====================
    m_procHandler->startInputProcess();
    if (m_procHandler->isProcess(ProcType::c_Input)) {   // input process gets the path of input modules
      if (inputPath and not inputPath->isEmpty()) {
        localPath = inputPath;
      }
    } else {
      // This is not the input process, clean up datastore to not contain the first event
      DataStore::Instance().invalidateData(DataStore::c_Event);

      // =================================
      // 5. Fork out output path
      // =================================
      m_procHandler->startOutputProcess();
      if (m_procHandler->isProcess(ProcType::c_Output)) {
        if (outputPath and not outputPath->isEmpty()) {
          localPath = outputPath;
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
            localPath = mainPath;
            if (inputPath and not inputPath->isEmpty()) {
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
    B2RESULT("Running as " << m_procHandler->getProcessName());

    // self healing mode will restart died workers
    bool selfHealing = true;
    // Monitoring process ignores SIGINT, SIGTERM, SIGQUIT
    B2INFO("Waiting for all processes to finish.");
    if (selfHealing) {
      while (m_procHandler->checkProcessStatus() && gSignalReceived == 0) {
        bool workerDied = m_procHandler->proceedPCBMulticast();
        // ===========================================
        // 7. Restart died workers
        // ===========================================
        if (workerDied) {
          B2RESULT("Restart worker");
          m_procHandler->restartWorkerProcess();
          if (m_procHandler->isProcess(ProcType::c_Worker)) {
            localPath = mainPath;
            if (inputPath and not inputPath->isEmpty()) {
              m_master = localPath->getModules().begin()->get(); //set Rx as master
            }
            break;
          }
        }
      }
    } else {
      m_procHandler->waitForAllProcesses();
    }
  } else
    installMainSignalHandlers(); // Main signals have no effect, still do this in prochandler::startproc



  // This is very all processes and up:
  if (not m_procHandler->isProcess(ProcType::c_Output) and not m_procHandler->isProcess(ProcType::c_Monitor)) {
    DataStoreStreamer::removeSideEffects();
  }

  bool gotSigINT = false;

  if (localPath != nullptr) { // if not monitoring process then process the module paths
    B2RESULT("Running as " << m_procHandler->getProcessName());
    ModulePtrList localModules = localPath->buildModulePathList();
    try {
      // ======================================
      // 8. here all the modules are processed
      // ======================================

      maxEvent = getMaximumEventNumber(maxEvent);
      processCore(localPath, localModules, maxEvent, m_procHandler->isProcess(ProcType::c_Input));

      B2INFO("After process core");
    } catch (StoppedBySignalException& e) {
      if (e.signal != SIGINT) {
        B2FATAL(e.what());
      }
      B2INFO("Stopped by exception");
      //in case of SIGINT, we move on to processTerminate() to shut down safely
      gotSigINT = true;
    }


    B2DEBUG(100, "terminate process...");
    PathUtils::prependModulesIfNotPresent(&localModules, terminateGlobally);
    // process the safe shutdown
    processTerminate(localModules);
  }

  // ============================================
  // 9. all processes stop here except monitor
  // ============================================
  if (not m_procHandler->isProcess(ProcType::c_Monitor)) {
    B2INFO(m_procHandler->getProcessName() << " process finished.");
    exit(0);
  }

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
    if (gSignalReceived == SIGINT) {
      B2RESULT("Processing aborted via signal " << gSignalReceived <<
               ", terminating. Output files have been closed safely and should be readable.");
    } else {
      B2ERROR("Processing aborted via signal " << gSignalReceived <<
              ", terminating. Output files have been closed safely and should be readable.");
    }
    installSignalHandler(gSignalReceived, SIG_DFL);
    raise(gSignalReceived);
  }
}

void pEventProcessor::sendPCBMessage(const c_MessageTypes msgType, const std::string& data)
{
  m_procHandler->sendPCBMessage(msgType, data);
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

    PathUtils::prependModulesIfNotPresent(modules, prependModules);
    // process the safe shutdown
    processTerminate(*modules);

    m_procHandler->killAllChildProc(); // shutdown Proxy and kill what in the pid list
  }

}





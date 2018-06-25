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

#include <framework/pcore/zmq/proxy/ZMQMulticastProxy.h>

#include <thread>

#include <TROOT.h>

#include <chrono>
#include <thread>

#include <signal.h>
#include <fstream>

using namespace std;
using namespace Belle2;

namespace {
  /**
   * Signal handlers do not work with member functions. So we need to "wrap" them in this top level function.
   * Unfortunately, we will then loose the object information, so we store it in this global variable (eval!)
   * We also store the signal we have received here
   */

  /// Received signal via a signal handler
  static int g_signalReceived = 0;

  /// Memory for the current (single!) instance for the signal handler
  static pEventProcessor* g_eventProcessorForSignalHandling = nullptr;

  static void cleanupAndRaiseSignal(int signalNumber)
  {
    if (g_eventProcessorForSignalHandling) {
      g_eventProcessorForSignalHandling->cleanup();
    }
    // uninstall current handler and call default one.
    signal(signalNumber, SIG_DFL);
    raise(signalNumber);
  }

  static void cleanupAndStoreSignal(int signalNumber)
  {
    if (signalNumber == SIGINT) {
      EventProcessor::writeToStdErr("\nStopping basf2 gracefully...\n");
      if (g_eventProcessorForSignalHandling) {
        g_eventProcessorForSignalHandling->cleanup();
      }
    }

    // Well, what do we do in the other cases? We probably just die...

    // We do not want to remove the first signal
    if (g_signalReceived == 0) {
      g_signalReceived = signalNumber;
    }
  }
} // namespace

pEventProcessor::pEventProcessor(const std::string& socketAddress) : EventProcessor(),
  m_socketAddress(socketAddress)
{
  B2ASSERT("You are having two instances of the pEventProcessor running! This is not possible",
           not g_eventProcessorForSignalHandling);
  g_eventProcessorForSignalHandling = this;
}

pEventProcessor::~pEventProcessor()
{
  std::cerr << "Called destructor in " << ProcHandler::EvtProcID() << std::endl;
  cleanup();
  g_eventProcessorForSignalHandling = nullptr;
}

void pEventProcessor::process(PathPtr path, long maxEvent)
{
  // Concerning signal handling:
  // * During the initialization, we just raise the signal without doing any cleanup etc.
  // * During the event execution, we will not allow for any signal in all processes except the parent process.
  //   Here, we catch sigint and clean up the processes AND WHAT DO WE DO IN THE OTHER CASES?
  // * During cleanup, we will just ignore sigint, but the rest will be raised

  if (path->isEmpty()) {
    return;
  }

  const int numProcesses = Environment::Instance().getNumberProcesses();
  if (numProcesses == 0) {
    B2FATAL("pEventProcessor::process() called for serial processing! Most likely a bug in Framework.");
  }

  // Split the path into input, main and output. A nullptr means, the path should not be used
  PathPtr inputPath, mainPath, outputPath;
  std::tie(inputPath, mainPath, outputPath) = PathUtils::splitPath(path);
  const ModulePtr& histogramManager = PathUtils::getHistogramManager(inputPath, mainPath, outputPath);

  if (not mainPath or mainPath->isEmpty()) {
    B2WARNING("Cannot run any modules in parallel (no c_ParallelProcessingCertified flag), falling back to single-core mode.");
    EventProcessor::process(path, maxEvent);
    return;
  }

  // inserts Rx/Tx modules into path (sets up IPC structures)
  const ModulePtrList& moduleList = PathUtils::preparePaths(inputPath, mainPath, outputPath, m_socketAddress);

  B2DEBUG(10, "Initialisation phase");
  // Run the initialization of the modules and the histogram manager
  initialize(moduleList, histogramManager);

  B2DEBUG(10, "Main phase");
  // The main part: fork into the different processes and run!
  const ModulePtrList& terminateGlobally = PathUtils::getTerminateGloballyModules(moduleList);
  forkAndRun(maxEvent, inputPath, mainPath, outputPath, terminateGlobally);

  B2DEBUG(10, "Terminate phase");
  // No matter what the user does, we want to do this cleanup...
  installMainSignalHandlers(cleanupAndRaiseSignal);
  installSignalHandler(SIGINT, SIG_IGN);
  // Run the final termination and cleanup with error check
  terminateAndCleanup(histogramManager);
}

void pEventProcessor::initialize(const ModulePtrList& moduleList, const ModulePtr& histogramManager)
{
  if (histogramManager) {
    histogramManager->initialize();
  }
  // from now on the datastore is available
  processInitialize(moduleList, true);

  // Don't start processing in case of no master module
  if (!m_master) {
    B2ERROR("There is no module that provides event and run numbers. You must either add the EventInfoSetter module to your path, or, if using an input module, read EventMetaData objects from file.");
  }

  // Check if errors appeared. If yes, don't start the event processing.
  int numLogError = LogSystem::Instance().getMessageCounter(LogConfig::c_Error);
  if (numLogError != 0) {
    B2FATAL(numLogError << " ERROR(S) occurred! The processing of events will not be started.");
  }

  // TODO: I do not really understand what is going on here...
  /** TFiles are stored in a global list and cleaned up by root
   * since this will happen in all forked processes, these will be corrupted if we don't clean the list!
   *
   * needs to be called at the end of every process.
   */
  // disable ROOT's management of TFiles
  // clear list, but don't actually delete the objects
  gROOT->GetListOfFiles()->Clear("nodelete");
}

void pEventProcessor::terminateAndCleanup(const ModulePtr& histogramManager)
{
  cleanup();

  if (histogramManager) {
    B2INFO("HistoManager:: adding histogram files");
    RbTupleManager::Instance().hadd();
  }

  // did anything bad happen?
  if (g_signalReceived) {
    if (g_signalReceived == SIGINT) {
      B2RESULT("Processing aborted via signal " << g_signalReceived <<
               ", terminating. Output files have been closed safely and should be readable.");
    } else {
      B2ERROR("Processing aborted via signal " << g_signalReceived <<
              ", terminating. Output files have been closed safely and should be readable.");
    }
    // re-raise the signal
    installSignalHandler(g_signalReceived, SIG_DFL);
    raise(g_signalReceived);
  }
}

void pEventProcessor::runInput(const PathPtr& inputPath, const ModulePtrList& terminateGlobally, long maxEvent)
{
  if (not inputPath or inputPath->isEmpty()) {
    return;
  }

  if (not ProcHandler::startInputProcess()) {
    // This is not the input process, clean up datastore to not contain the first event
    DataStore::Instance().invalidateData(DataStore::c_Event);
    return;
  }

  DataStoreStreamer::removeSideEffects();

  processPath(inputPath, terminateGlobally, maxEvent);
  B2DEBUG(10, "Finished an input process");
  exit(0);
}

void pEventProcessor::runOutput(const PathPtr& outputPath, const ModulePtrList& terminateGlobally, long maxEvent)
{
  if (not outputPath or outputPath->isEmpty()) {
    return;
  }

  if (not ProcHandler::startOutputProcess()) {
    return;
  }

  // Set the rx module as main module
  m_master = outputPath->getModules().begin()->get();

  processPath(outputPath, terminateGlobally, maxEvent);
  B2DEBUG(10, "Finished an output process");
  exit(0);
}
void pEventProcessor::runWorker(unsigned int numProcesses, const PathPtr& inputPath, const PathPtr& mainPath,
                                const ModulePtrList& terminateGlobally,
                                long maxEvent)
{
  if (numProcesses == 0) {
    return;
  }

  if (not ProcHandler::startWorkerProcesses(numProcesses)) {
    // Make sure the worker process is running until we go on
    m_processMonitor.waitForRunningWorker(4);
    return;
  }

  if (inputPath and not inputPath->isEmpty()) {
    // set Rx as master
    m_master = mainPath->getModules().begin()->get();
  }

  // No matter what happens, we do not want to stop the execution
  // TODO: or do we only want to do this on SIGINT???
  installMainSignalHandlers(SIG_IGN);

  DataStoreStreamer::removeSideEffects();

  processPath(mainPath, terminateGlobally, maxEvent);
  B2DEBUG(10, "Finished a worker process");
  exit(0);
}

void pEventProcessor::processPath(const PathPtr& localPath, const ModulePtrList& terminateGlobally, long maxEvent)
{
  ModulePtrList localModules = localPath->buildModulePathList();
  maxEvent = getMaximumEventNumber(maxEvent);
  // we are not using the default signal handler, so the processCore can not throw any exception because if sigint...
  processCore(localPath, localModules, maxEvent, ProcHandler::isProcess(ProcType::c_Input));

  B2DEBUG(10, "terminate process...");
  PathUtils::prependModulesIfNotPresent(&localModules, terminateGlobally);
  processTerminate(localModules);
}


void pEventProcessor::runMonitoring(const PathPtr& inputPath, const PathPtr& mainPath, const ModulePtrList& terminateGlobally,
                                    long maxEvent)
{
  if (not ProcHandler::startMonitoringProcess()) {
    return;
  }

  // We catch all signals and store them into a variable. This is used during the main loop then.
  installMainSignalHandlers(cleanupAndStoreSignal);

  B2DEBUG(10, "Will now start process monitor...");
  const int numProcesses = Environment::Instance().getNumberProcesses();
  m_processMonitor.initialize(numProcesses);

  // Make sure the input process is running until we go on
  m_processMonitor.waitForRunningInput(4);
  // Make sure the output process is running until we go on
  m_processMonitor.waitForRunningOutput(4);

  B2DEBUG(10, "Will now start main loop...");
  while (true) {
    // check multicast for messages and kill workers if requested
    m_processMonitor.checkMulticast();
    // check the child processes, if one has died
    m_processMonitor.checkChildProcesses();
    // check if we have received any signal from the user or OS. Kill the processes if not SIGINT.
    m_processMonitor.checkSignals(g_signalReceived);

    // If we have received a SIGINT signal or the last process is gone, we can end smoothly
    if (m_processMonitor.hasEnded()) {
      break;
    }

    runWorker(m_processMonitor.needMoreWorkers(), inputPath, mainPath, terminateGlobally, maxEvent);
  }

  B2DEBUG(10, "Finished the monitoring process");
}

void pEventProcessor::forkAndRun(long maxEvent, const PathPtr& inputPath, const PathPtr& mainPath, const PathPtr& outputPath,
                                 const ModulePtrList& terminateGlobally)
{
  const int numProcesses = Environment::Instance().getNumberProcesses();
  ProcHandler::initialize(numProcesses);

  const auto pubSocketAddress(ZMQHelper::getSocketAddress(m_socketAddress, ZMQAddressType::c_pub));
  const auto subSocketAddress(ZMQHelper::getSocketAddress(m_socketAddress, ZMQAddressType::c_sub));
  const auto controlSocketAddress(ZMQHelper::getSocketAddress(m_socketAddress, ZMQAddressType::c_control));

  // The default will be to not do anything on signals...
  // TODO: or do we only want to do this on SIGINT???
  installMainSignalHandlers(SIG_IGN);

  m_processMonitor.subscribe(pubSocketAddress, subSocketAddress, controlSocketAddress);

  B2DEBUG(10, "Starting input process...");
  runInput(inputPath, terminateGlobally, maxEvent);
  B2DEBUG(10, "Starting output process...");
  runOutput(outputPath, terminateGlobally, maxEvent);
  B2DEBUG(10, "Starting monitoring process...");
  runMonitoring(inputPath, mainPath, terminateGlobally, maxEvent);
}

void pEventProcessor::cleanup()
{
  if (not ProcHandler::isProcess(ProcType::c_Monitor) and not ProcHandler::isProcess(ProcType::c_Init)) {
    B2DEBUG(10, "Not running cleanup, as I am in process type " << ProcHandler::getProcessName());
    return;
  }
  std::cerr << "Running cleanup in " << ProcHandler::getProcessName() << std::endl;
  std::cerr << "Trying to kill every process" << std::endl;
  m_processMonitor.killProcesses(5);
  m_processMonitor.terminate();
  // TODO: make sure to clean up the ZMQ resources
}
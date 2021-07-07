/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/pcore/ProcHelper.h>
#include <framework/pcore/GlobalProcHandler.h>
#include <framework/pcore/zmq/messages/ZMQDefinitions.h>
#include <framework/pcore/zmq/utils/ZMQAddressUtils.h>
#include <framework/pcore/zmq/messages/ZMQMessageFactory.h>
#include <framework/pcore/PathUtils.h>

#include <framework/pcore/ZMQEventProcessor.h>
#include <framework/pcore/DataStoreStreamer.h>
#include <framework/pcore/RbTuple.h>

#include <framework/core/Environment.h>
#include <framework/logging/LogSystem.h>

#include <TROOT.h>

#include <sys/stat.h>

#include <csignal>
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
  static ZMQEventProcessor* g_eventProcessorForSignalHandling = nullptr;

  static void cleanupAndRaiseSignal(int signalNumber)
  {
    if (g_eventProcessorForSignalHandling) {
      g_eventProcessorForSignalHandling->cleanup();
    }
    // uninstall current handler and call default one.
    signal(signalNumber, SIG_DFL);
    raise(signalNumber);
  }

  static void storeSignal(int signalNumber)
  {
    if (signalNumber == SIGINT) {
      EventProcessor::writeToStdErr("\nStopping basf2 gracefully...\n");
    }

    // We do not want to remove the first signal
    if (g_signalReceived == 0) {
      g_signalReceived = signalNumber;
    }
  }

  /// The socket address currently in use
  std::string g_socketAddress = "";

  void deleteSocketFiles()
  {
    if (not GlobalProcHandler::isProcess(ProcType::c_Monitor) and not GlobalProcHandler::isProcess(ProcType::c_Init)) {
      return;
    }

    const std::vector<ZMQAddressType> socketAddressList = {ZMQAddressType::c_input, ZMQAddressType::c_output, ZMQAddressType::c_pub, ZMQAddressType::c_sub, ZMQAddressType::c_control};
    const auto seperatorPos = g_socketAddress.find("://");

    if (seperatorPos == std::string::npos or seperatorPos + 3 >= g_socketAddress.size()) {
      return;
    }

    const std::string filename(g_socketAddress.substr(seperatorPos + 3));

    struct stat buffer;
    for (const auto socketAdressType : socketAddressList) {
      const std::string socketAddress(ZMQAddressUtils::getSocketAddress(filename, socketAdressType));
      if (stat(socketAddress.c_str(), &buffer) == 0) {
        remove(socketAddress.c_str());
      }
    }
  }
} // namespace

ZMQEventProcessor::ZMQEventProcessor()
{
  B2ASSERT("You are having two instances of the ZMQEventProcessor running! This is not possible",
           not g_eventProcessorForSignalHandling);
  g_eventProcessorForSignalHandling = this;

  // Make sure to remove the sockets
  g_socketAddress = Environment::Instance().getZMQSocketAddress();
  std::atexit(deleteSocketFiles);
}

ZMQEventProcessor::~ZMQEventProcessor()
{
  cleanup();
  g_eventProcessorForSignalHandling = nullptr;
}

void ZMQEventProcessor::process(const PathPtr& path, long maxEvent)
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
    B2FATAL("ZMQEventProcessor::process() called for serial processing! Most likely a bug in Framework.");
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
  const ModulePtrList& moduleList = PathUtils::preparePaths(inputPath, mainPath, outputPath);

  B2DEBUG(10, "Initialisation phase");
  // Run the initialization of the modules and the histogram manager
  initialize(moduleList, histogramManager);

  B2DEBUG(10, "Main phase");
  // The main part: fork into the different processes and run!
  const ModulePtrList& terminateGlobally = PathUtils::getTerminateGloballyModules(moduleList);
  forkAndRun(maxEvent, inputPath, mainPath, outputPath, terminateGlobally);

  B2DEBUG(10, "Terminate phase");
  installMainSignalHandlers(cleanupAndRaiseSignal);
  // Run the final termination and cleanup with error check
  terminateAndCleanup(histogramManager);
}

void ZMQEventProcessor::initialize(const ModulePtrList& moduleList, const ModulePtr& histogramManager)
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

void ZMQEventProcessor::terminateAndCleanup(const ModulePtr& histogramManager)
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

void ZMQEventProcessor::runInput(const PathPtr& inputPath, const ModulePtrList& terminateGlobally, long maxEvent)
{
  if (not inputPath or inputPath->isEmpty()) {
    return;
  }

  if (not GlobalProcHandler::startInputProcess()) {
    // This is not the input process, clean up datastore to not contain the first event
    DataStore::Instance().invalidateData(DataStore::c_Event);
    return;
  }

  // The default will be to not do anything on signals...
  installMainSignalHandlers(SIG_IGN);

  m_processMonitor.reset();
  DataStoreStreamer::removeSideEffects();

  processPath(inputPath, terminateGlobally, maxEvent);
  B2DEBUG(10, "Finished an input process");
  exit(0);
}

void ZMQEventProcessor::runOutput(const PathPtr& outputPath, const ModulePtrList& terminateGlobally, long maxEvent)
{
  const auto& socketAddress = Environment::Instance().getZMQSocketAddress();
  const auto pubSocketAddress(ZMQAddressUtils::getSocketAddress(socketAddress, ZMQAddressType::c_pub));
  const auto subSocketAddress(ZMQAddressUtils::getSocketAddress(socketAddress, ZMQAddressType::c_sub));

  if (not outputPath or outputPath->isEmpty()) {
    return;
  }

  if (not GlobalProcHandler::startOutputProcess()) {
    return;
  }

  // The default will be to not do anything on signals...
  installMainSignalHandlers(SIG_IGN);

  m_processMonitor.reset();

  // Set the rx module as main module
  m_master = outputPath->getModules().begin()->get();

  processPath(outputPath, terminateGlobally, maxEvent);

  // Send the statistics to the process monitor
  StreamHelper streamer;
  ZMQClient zmqClient;

  // TODO: true?
  streamer.initialize(0, true);
  zmqClient.initialize(pubSocketAddress, subSocketAddress);

  // TODO: make sure to only send statistics!
  const auto& evtMessage = streamer.stream();
  auto message = ZMQMessageFactory::createMessage(EMessageTypes::c_statisticMessage, evtMessage);
  zmqClient.publish(std::move(message));

  B2DEBUG(10, "Finished an output process");
  exit(0);
}
void ZMQEventProcessor::runWorker(unsigned int numProcesses, const PathPtr& inputPath, const PathPtr& mainPath,
                                  const ModulePtrList& terminateGlobally, long maxEvent)
{
  if (numProcesses == 0) {
    return;
  }

  if (not GlobalProcHandler::startWorkerProcesses(numProcesses)) {
    // Make sure the worker process is running until we go on
    m_processMonitor.waitForRunningWorker(60);
    return;
  }

  // The default will be to not do anything on signals...
  installMainSignalHandlers(SIG_IGN);

  if (inputPath and not inputPath->isEmpty()) {
    // set Rx as master
    m_master = mainPath->getModules().begin()->get();
  }

  m_processMonitor.reset();
  DataStoreStreamer::removeSideEffects();

  processPath(mainPath, terminateGlobally, maxEvent);
  B2DEBUG(10, "Finished a worker process");
  exit(0);
}

void ZMQEventProcessor::processPath(const PathPtr& localPath, const ModulePtrList& terminateGlobally, long maxEvent)
{
  ModulePtrList localModules = localPath->buildModulePathList();
  maxEvent = getMaximumEventNumber(maxEvent);
  // we are not using the default signal handler, so the processCore can not throw any exception because if sigint...
  processCore(localPath, localModules, maxEvent, GlobalProcHandler::isProcess(ProcType::c_Input));

  B2DEBUG(10, "terminate process...");
  PathUtils::prependModulesIfNotPresent(&localModules, terminateGlobally);
  processTerminate(localModules);
}


void ZMQEventProcessor::runMonitoring(const PathPtr& inputPath, const PathPtr& mainPath, const ModulePtrList& terminateGlobally,
                                      long maxEvent)
{
  if (not GlobalProcHandler::startMonitoringProcess()) {
    return;
  }

  const auto& environment = Environment::Instance();

  B2DEBUG(10, "Will now start process monitor...");
  const int numProcesses = environment.getNumberProcesses();
  m_processMonitor.initialize(numProcesses);

  // Make sure the input process is running until we go on
  m_processMonitor.waitForRunningInput(60);
  if (m_processMonitor.hasEnded()) {
    return;
  }
  // Make sure the output process is running until we go on
  m_processMonitor.waitForRunningOutput(60);
  if (m_processMonitor.hasEnded()) {
    return;
  }

  installMainSignalHandlers(storeSignal);

  // at least start the number of workers requested
  runWorker(m_processMonitor.needMoreWorkers(), inputPath, mainPath, terminateGlobally, maxEvent);

  const auto& restartFailedWorkers = environment.getZMQRestartFailedWorkers();
  const auto& failOnFailedWorkers = environment.getZMQFailOnFailedWorkers();

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

    // Test if we need more workers
    const unsigned int neededWorkers = m_processMonitor.needMoreWorkers();
    if (neededWorkers > 0) {
      if (restartFailedWorkers) {
        runWorker(neededWorkers, inputPath, mainPath, terminateGlobally, maxEvent);
      } else if (failOnFailedWorkers) {
        B2ERROR("A worker failed. Will try to end the process smoothly now.");
        break;
      } else if (not m_processMonitor.hasWorkers()) {
        B2WARNING("All workers have died and you did not request to restart them. Going down now.");
        break;
      }
    }
  }

  B2DEBUG(10, "Finished the monitoring process");
}

void ZMQEventProcessor::forkAndRun(long maxEvent, const PathPtr& inputPath, const PathPtr& mainPath, const PathPtr& outputPath,
                                   const ModulePtrList& terminateGlobally)
{
  const int numProcesses = Environment::Instance().getNumberProcesses();
  GlobalProcHandler::initialize(numProcesses);

  const auto& socketAddress = Environment::Instance().getZMQSocketAddress();

  const auto pubSocketAddress(ZMQAddressUtils::getSocketAddress(socketAddress, ZMQAddressType::c_pub));
  const auto subSocketAddress(ZMQAddressUtils::getSocketAddress(socketAddress, ZMQAddressType::c_sub));
  const auto controlSocketAddress(ZMQAddressUtils::getSocketAddress(socketAddress, ZMQAddressType::c_control));

  // We catch all signals and store them into a variable. This is used during the main loop then.
  // From now on, we have to make sure to clean up behind us
  installMainSignalHandlers(cleanupAndRaiseSignal);
  m_processMonitor.subscribe(pubSocketAddress, subSocketAddress, controlSocketAddress);

  B2DEBUG(10, "Starting input process...");
  runInput(inputPath, terminateGlobally, maxEvent);
  B2DEBUG(10, "Starting output process...");
  runOutput(outputPath, terminateGlobally, maxEvent);

  B2DEBUG(10, "Starting monitoring process...");
  runMonitoring(inputPath, mainPath, terminateGlobally, maxEvent);
}

void ZMQEventProcessor::cleanup()
{
  if (not GlobalProcHandler::isProcess(ProcType::c_Monitor) and not GlobalProcHandler::isProcess(ProcType::c_Init)) {
    B2DEBUG(10, "Not running cleanup, as I am in process type " << GlobalProcHandler::getProcessName());
    return;
  }
  m_processMonitor.killProcesses(5);
  m_processMonitor.terminate();

  deleteSocketFiles();
}

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

#include <framework/database/DBStore.h>
#include <framework/database/Database.h>
#include <framework/core/RandomNumbers.h>
#include <framework/core/MetadataService.h>
#include <framework/gearbox/Unit.h>
#include <framework/utilities/Utils.h>

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

  /// DAQ environment
  static bool g_daq_environment = false;

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

  // Check for existence of HLTZMQ2Ds module in input path to set DAQ environment
  for (const ModulePtr& module : inputPath->getModules()) {
    if (module->getName() == "HLTZMQ2Ds") {
      g_daq_environment = true;
      Environment::Instance().setZMQDAQEnvironment(g_daq_environment);
      B2INFO("ZMQEventProcessor : DAQ environment set");
      break;
    }
  }

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

  B2INFO("ZMQEventProcessor : processInitialize done");

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
    m_processMonitor.waitForRunningWorker(7200);
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
  processCore(localPath, localModules, maxEvent, GlobalProcHandler::isProcess(ProcType::c_Input),
              GlobalProcHandler::isProcess(ProcType::c_Worker),
              GlobalProcHandler::isProcess(ProcType::c_Output));

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

  runInput(inputPath, terminateGlobally, maxEvent);
  runOutput(outputPath, terminateGlobally, maxEvent);
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

void ZMQEventProcessor::processCore(const PathPtr& startPath, const ModulePtrList& modulePathList, long maxEvent,
                                    bool isInputProcess, bool isWorkerProcess, bool isOutputProcess)
{
  DataStore::Instance().setInitializeActive(false);
  m_moduleList = modulePathList;

  //Remember the previous event meta data, and identify end of data meta data
  m_previousEventMetaData.setEndOfData(); //invalid start state

  const bool collectStats = !Environment::Instance().getNoStats();

  //Loop over the events
  long currEvent = 0;
  bool endProcess = false;
  while (!endProcess) {
    if (collectStats)
      m_processStatisticsPtr->startGlobal();

    PathIterator moduleIter(startPath);

    if (isInputProcess) {
      endProcess = ZMQEventProcessor::processEvent(moduleIter, isInputProcess && currEvent == 0);
    } else if (isWorkerProcess) {
      endProcess = ZMQEventProcessor::processEvent(moduleIter, false, isWorkerProcess && currEvent == 0 && g_daq_environment);
    } else if (isOutputProcess) {
      endProcess = ZMQEventProcessor::processEvent(moduleIter, false, false, isOutputProcess && currEvent == 0 && g_daq_environment);
    } else {
      B2ERROR("processCore : should not come here. Specified path is invalid");
      return;
    }

    //Delete event related data in DataStore
    DataStore::Instance().invalidateData(DataStore::c_Event);

    currEvent++;
    if ((maxEvent > 0) && (currEvent >= maxEvent)) endProcess = true;
    if (collectStats)
      m_processStatisticsPtr->stopGlobal(ModuleStatistics::c_Event);

    //    firstRound = false;

    //    B2INFO ( "processCore :: event processed" );

  } //end event loop

  //End last run
  m_eventMetaDataPtr.create();
  B2INFO("processCore : End Last Run. calling processEndRun()");
  processEndRun();
}


bool ZMQEventProcessor::processEvent(PathIterator moduleIter, bool skipMasterModule, bool WorkerPath, bool OutputPath)
{
  double time = Utils::getClock() / Unit::s;
  if (time > m_lastMetadataUpdate + m_metadataUpdateInterval) {
    MetadataService::Instance().addBasf2Status("running event loop");
    m_lastMetadataUpdate = time;
  }

  const bool collectStats = !Environment::Instance().getNoStats();

  while (!moduleIter.isDone()) {
    Module* module = moduleIter.get();
    //    B2INFO ("Starting event of " << module->getName() );

    // run the module ... unless we don't want to
    if (module != m_master) {
      callEvent(module);
      //      B2INFO ( "not master. callEvent" );
      //      B2INFO ( "ZMQEventProcessor :: " <<module->getName() << " called. Not master" );
    } else if (!skipMasterModule) {
      callEvent(module);
      //      B2INFO ( "master but not skipModule. callEvent");
      //      B2INFO ( "ZMQEventProcessor :: " <<module->getName() << " called. Not skipMasterModule" );
    } else
      B2INFO("Skipping execution of module " << module->getName());

    if (!m_eventMetaDataPtr) {
      //      B2INFO ( "No event metadata....." );
      return false;
    }

    //Check for end of data
    if (m_eventMetaDataPtr->isEndOfData()) {
      // Immediately leave the loop and terminate (true)
      B2INFO("isEndOfData. Return");
      return true;
    }

    //Handle EventMetaData changes by master module
    if (module == m_master && !skipMasterModule) {

      //initialize random number state for the event
      RandomNumbers::initializeEvent();

      // Worker Path
      if (WorkerPath) {
        B2INFO("Worker Path and First Event!");
        if (Environment::Instance().isZMQDAQFirstEvent(m_eventMetaDataPtr->getExperiment(), m_eventMetaDataPtr->getRun())) {
          //  if ( m_eventMetaDataPtr->getExperiment() == Environment::Instance().getZMQDAQFirstEventExp() &&
          //       m_eventMetaDataPtr->getRun() == Environment::Instance().getZMQDAQFirstEventRun() ) {
          B2ERROR("Worker path processing for ZMQDAQ first event.....Skip to the end of path");
          B2ERROR("    --> exp = " << m_eventMetaDataPtr->getExperiment() << " run = " << m_eventMetaDataPtr->getRun());
          while (true) {
            module = moduleIter.get();
            //    B2INFO ( "Module in the path = " << module->getName() );
            if (module->getName() == "ZMQTxWorker") break;
            moduleIter.next();
          }
          //  B2INFO ( "ZMQTxWorker will be called" );
          continue;
        }
      }

      // Check for EndOfRun
      if (!WorkerPath && !OutputPath) {
        if (m_eventMetaDataPtr->isEndOfRun()) {
          //        B2DEBUG(10, "===> EndOfRun : calling processEndRun(); isEndOfRun = " << m_eventMetaDataPtr->isEndOfRun());
          B2INFO("===> EndOfRun : calling processEndRun(); isEndOfRun = " << m_eventMetaDataPtr->isEndOfRun());
          processEndRun();
          // Store the current event meta data for the next round
          m_previousEventMetaData = *m_eventMetaDataPtr;
          // Leave this event, but not the full processing (false)
          return false;
        } else if (m_previousEventMetaData.isEndOfData() || m_previousEventMetaData.isEndOfRun()) {
          if (m_eventMetaDataPtr->getRun() ==  m_previousEventMetaData.getRun()) {
            B2INFO("===> EndOfData : ----> Run change request to the same run!!! Skip this event.");
            return false;
          }
          //        B2DEBUG(10, "===> EndOfData : calling processBeginRun(); isEndOfData = " << m_previousEventMetaData.isEndOfData());
          B2INFO("===> EndOfData : calling processBeginRun(); isEndOfData = " << m_previousEventMetaData.isEndOfData() <<
                 " isEndOfRun = " << m_previousEventMetaData.isEndOfRun());
          B2INFO("--> cur run = " << m_eventMetaDataPtr->getRun() << " <- prev run = " << m_previousEventMetaData.getRun());
          B2INFO("--> cur evt = " << m_eventMetaDataPtr->getEvent() << " <- prev evt = " << m_previousEventMetaData.getEvent());
          processBeginRun();
          m_previousEventMetaData = *m_eventMetaDataPtr;
        }

        //Check for a change of the run (should not come here)
        else {
          const bool runChanged = ((m_eventMetaDataPtr->getExperiment() != m_previousEventMetaData.getExperiment()) or
                                   (m_eventMetaDataPtr->getRun() != m_previousEventMetaData.getRun()));
          const bool runChangedWithoutNotice = runChanged and not m_previousEventMetaData.isEndOfData()
                                               and not m_previousEventMetaData.isEndOfRun();
          //  if (runChangedWithoutNotice && !g_first_round) {
          if (runChangedWithoutNotice) {
            if (collectStats)
              m_processStatisticsPtr->suspendGlobal();

            //        B2DEBUG(10, "EventProcessor : calling processEndRun() and processBeginRun() because of Run Change");
            B2INFO("===> Run Change (possibly offline) : calling processEndRun() and processBeginRun()");
            B2INFO("--> cur run = " << m_eventMetaDataPtr->getRun() << " <- prev run = " << m_previousEventMetaData.getRun());
            B2INFO("--> cur evt = " << m_eventMetaDataPtr->getEvent() << " <- prev evt = " << m_previousEventMetaData.getEvent());
            B2INFO("--> runChanged = " << runChanged << " runChangedWithoutNotice = " << runChangedWithoutNotice);

            processEndRun();
            processBeginRun();

            if (collectStats)
              m_processStatisticsPtr->resumeGlobal();
          }
        }
        m_previousEventMetaData = *m_eventMetaDataPtr;
      } else
        B2INFO("Skipping begin/end run processing");

      //make sure we use the event dependent generator again
      RandomNumbers::useEventDependent();

      DBStore::Instance().updateEvent();

    } else if (!WorkerPath && !OutputPath) {
      //Check for a second master module. Cannot do this if we skipped the
      //master module as the EventMetaData is probably set before we call this
      //function
      if (!skipMasterModule && m_eventMetaDataPtr &&
          (*m_eventMetaDataPtr != m_previousEventMetaData)) {
        B2FATAL("Two modules setting EventMetaData were discovered: " << m_master->getName() << " and " << module->getName());
      }
    }

    if (g_signalReceived != 0) {
      throw StoppedBySignalException(g_signalReceived);
    }

    //Check for the module conditions, evaluate them and if one is true switch to the new path
    if (module->evalCondition()) {
      PathPtr condPath = module->getConditionPath();
      //continue with parent Path after condition path is executed?
      if (module->getAfterConditionPath() == Module::EAfterConditionPath::c_Continue) {
        moduleIter = PathIterator(condPath, moduleIter);
      } else {
        moduleIter = PathIterator(condPath);
      }
    } else {
      moduleIter.next();
    }
  } //end module loop
  return false;
}

void ZMQEventProcessor::processBeginRun(bool skipDB)
{
  MetadataService::Instance().addBasf2Status("beginning run");

  m_inRun = true;
  auto dbsession = Database::Instance().createScopedUpdateSession();

  LogSystem& logSystem = LogSystem::Instance();
  m_processStatisticsPtr->startGlobal();

  if (!skipDB) DBStore::Instance().update();

  //initialize random generator for end run
  RandomNumbers::initializeBeginRun();

  for (const ModulePtr& modPtr : m_moduleList) {
    Module* module = modPtr.get();

    //Set the module dependent log level
    logSystem.updateModule(&(module->getLogConfig()), module->getName());

    //Do beginRun() call
    m_processStatisticsPtr->startModule();
    //    CALL_MODULE(module, beginRun);
    module->beginRun();
    m_processStatisticsPtr->stopModule(module, ModuleStatistics::c_BeginRun);

    //Set the global log level
    logSystem.updateModule(nullptr);
  }

  m_processStatisticsPtr->stopGlobal(ModuleStatistics::c_BeginRun);
}


void ZMQEventProcessor::processEndRun()
{
  MetadataService::Instance().addBasf2Status("ending run");

  if (!m_inRun)
    return;
  m_inRun = false;

  LogSystem& logSystem = LogSystem::Instance();
  m_processStatisticsPtr->startGlobal();

  const EventMetaData newEventMetaData = *m_eventMetaDataPtr;
  //  *m_eventMetaDataPtr = m_previousEventMetaData;

  //initialize random generator for end run
  RandomNumbers::initializeEndRun();

  for (const ModulePtr& modPtr : m_moduleList) {
    Module* module = modPtr.get();

    //Set the module dependent log level
    logSystem.updateModule(&(module->getLogConfig()), module->getName());

    //Do endRun() call
    m_processStatisticsPtr->startModule();
    //    CALL_MODULE(module, endRun);
    module->endRun();
    m_processStatisticsPtr->stopModule(module, ModuleStatistics::c_EndRun);

    //Set the global log level
    logSystem.updateModule(nullptr);
  }
  *m_eventMetaDataPtr = newEventMetaData;

  m_processStatisticsPtr->stopGlobal(ModuleStatistics::c_EndRun);
}

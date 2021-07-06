/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <daq/hbasf2/utils/HLTEventProcessor.h>

#include <boost/python.hpp>
#include <framework/utilities/RegisterPythonModule.h>
#include <framework/core/InputController.h>
#include <framework/pcore/ProcHandler.h>

#include <framework/database/DBStore.h>
#include <framework/core/RandomNumbers.h>
#include <framework/core/Environment.h>
#include <framework/core/ModuleManager.h>

#include <framework/pcore/zmq/messages/ZMQMessageFactory.h>
#include <framework/pcore/zmq/messages/ZMQDefinitions.h>
#include <framework/pcore/zmq/connections/ZMQConfirmedConnection.h>

#include <TROOT.h>

#include <sys/prctl.h>
#include <sys/wait.h>

#include <chrono>
#include <thread>
#include <signal.h>
#include <zmq.h>

using namespace Belle2;
using namespace boost::python;

namespace {
  /// Received signal via a signal handler
  static int g_signalReceived = 0;

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
}

void HLTEventProcessor::sendTerminatedMessage(unsigned int pid, bool waitForConformation)
{
  for (auto& socket : m_sockets) {
    auto message = ZMQMessageFactory::createMessage(EMessageTypes::c_deleteWorkerMessage,
                                                    ZMQParent::createIdentity(pid));
    ZMQParent::send(socket, std::move(message));

    if (not waitForConformation) {
      continue;
    }
    if (ZMQParent::poll({socket.get()}, 10 * 1000)) {
      auto acceptMessage = ZMQMessageFactory::fromSocket<ZMQNoIdMessage>(socket);
      B2ASSERT("Should be an accept message", acceptMessage->isMessage(EMessageTypes::c_confirmMessage));
    } else {
      B2FATAL("Did not receive a confirmation message!");
    }
  }
}

HLTEventProcessor::HLTEventProcessor(const std::vector<std::string>& outputAddresses)
{
  m_sockets.reserve(outputAddresses.size());
  for (const auto& address : outputAddresses) {
    m_sockets.push_back(m_parent.createSocket<ZMQ_DEALER>(address, false));
  }
}

void HLTEventProcessor::process(PathPtr path, bool restartFailedWorkers)
{
  using namespace std::chrono_literals;

  m_moduleList = path->buildModulePathList();

  // Assert path is what we want: fully parallel certified, not empty. Set first module to master module
  B2ASSERT("You try to process an empty path!", not m_moduleList.empty());
  for (const auto& module : m_moduleList) {
    bool hasParallelFlag = module->hasProperties(Module::c_ParallelProcessingCertified);
    // entire conditional path must also be compatible
    if (hasParallelFlag and module->hasCondition()) {
      for (const auto& conditionPath : module->getAllConditionPaths()) {
        if (!ModuleManager::allModulesHaveFlag(conditionPath->getModules(), Module::c_ParallelProcessingCertified)) {
          hasParallelFlag = false;
        }
      }
    }
    B2ASSERT("Module with name " << module->getName() << " does not have parallel flag!", hasParallelFlag);
  }

  // Initialize of all modules (including event() of master module)
  installMainSignalHandlers();
  processInitialize(m_moduleList);

  // Don't start processing in case of no master module
  if (not m_master) {
    B2ERROR("There is no module that provides event and run numbers (EventMetaData). "
            "You must add the specific HLT module as first module to the path.");
  }

  // Check if errors appeared. If yes, don't start the event processing.
  const int numLogError = LogSystem::Instance().getMessageCounter(LogConfig::c_Error);
  if (numLogError != 0) {
    B2FATAL(numLogError << " ERROR(S) occurred! The processing of events will not be started.");
  }

  // Start the workers, which call the main loop
  const int numProcesses = Environment::Instance().getNumberProcesses();
  runWorkers(path, numProcesses);

  installMainSignalHandlers(storeSignal);
  // Back in the main process: wait for the processes and monitor them
  int numberOfRestartedWorkers = 0;
  while (true) {
    // check if we have received any signal from the user or OS.
    // Killing of the remaining processes happens after the loop.
    if (g_signalReceived > 0) {
      B2WARNING("Received a signal to go down.");
      break;
    }

    // Test if we need more workers and if one has died
    unsigned int presentWorkers;
    unsigned int neededWorkers;

    std::tie(presentWorkers, neededWorkers) = checkChildProcesses();
    if (neededWorkers > 0) {
      if (restartFailedWorkers) {
        runWorkers(path, neededWorkers);
        numberOfRestartedWorkers += neededWorkers;
      } else {
        B2ERROR("A worker failed. Will try to end the process smoothly now.");
        break;
      }
    } else if (presentWorkers == 0) {
      B2DEBUG(10, "All workers have cleanly exited. Will now also exit");
      break;
    }

    if (numberOfRestartedWorkers > numProcesses) {
      B2ERROR("I needed to restart on total " << numberOfRestartedWorkers << ", which I think is abnormal. "
              "Will terminate the process now!");
      break;
    }

    std::this_thread::sleep_for(10ms);
  }

  checkChildProcesses();

  // if we still have/had processes, we should unregister them
  std::this_thread::sleep_for(500ms);

  for (const int& pid : m_processList) {
    if (kill(pid, SIGKILL) >= 0) {
      B2WARNING("Needed to hard kill process " << pid);
    } else {
      B2DEBUG(100, "no process " << pid << " found, already gone?");
    }
    sendTerminatedMessage(pid, false);
  }
  m_processList.clear();

  B2DEBUG(10, "Done here");

  // Normally, we would call terminate here, but not on HLT!
  // Normally, we would print the error summary here, but not on HLT!
  if (g_signalReceived == SIGINT) {
    installSignalHandler(SIGINT, SIG_DFL);
    raise(SIGINT);
  }
}

void HLTEventProcessor::runWorkers(PathPtr path, unsigned int numProcesses)
{
  for (unsigned int i = 0; i < numProcesses; i++) {
    if (forkOut()) {
      // Do only run in forked out worker process:
      B2DEBUG(10, "Starting a new worker process");
      // Reset the parent and sockets
      release();

      // Start the main loop with our signal handling and error catching
      installMainSignalHandlers(storeSignal);
      try {
        processCore(path);
      } catch (StoppedBySignalException& e) {
        // close all open ROOT files, ROOT's exit handler will crash otherwise
        gROOT->GetListOfFiles()->Delete();

        B2ERROR(e.what());
        exit(1);
      } catch (...) {
        if (m_eventMetaDataPtr)
          B2ERROR("Exception occured in exp/run/evt: "
                  << m_eventMetaDataPtr->getExperiment() << " / "
                  << m_eventMetaDataPtr->getRun() << " / "
                  << m_eventMetaDataPtr->getEvent());
        throw;
      }

      B2DEBUG(10, "Ending a worker process here.");
      // Ok, we are done here!
      exit(0);
    }
  }
}

void HLTEventProcessor::processCore(PathPtr path)
{
  bool terminationRequested = false;
  bool firstRound = true;

  // Initialisation is done
  DataStore::Instance().setInitializeActive(false);

  // Set the previous event meta data to something invalid
  m_previousEventMetaData.setEndOfData();

  while (not terminationRequested) {
    B2DEBUG(100, "Processing new event");

    // Start the measurement
    m_processStatisticsPtr->startGlobal();

    // Main call to event() of the modules, and maybe beginRun() and endRun()
    PathIterator moduleIter(path);
    terminationRequested = processEvent(moduleIter, firstRound);

    // Delete event related data in DataStore
    DataStore::Instance().invalidateData(DataStore::c_Event);

    // Stop the measurement
    m_processStatisticsPtr->stopGlobal(ModuleStatistics::c_Event);

    // We are surely not in the first round the next time
    firstRound = false;
  }

  // End last run with a terminate. Yes, we are not calling a endRun() here and yes, we are calling this in the worker
  B2DEBUG(10, "Calling terminate");
  m_eventMetaDataPtr.create();
  processTerminate(m_moduleList);
}

bool HLTEventProcessor::processEvent(PathIterator moduleIter, bool firstRound)
{
  while (not moduleIter.isDone()) {
    Module* module = moduleIter.get();
    B2DEBUG(10, "Starting event of " << module->getName());

    // The actual call of the event function
    if (module != m_master) {
      // If this is not the master module it is quite simple: just call the event function
      callEvent(module);

      // Check for a second master module. Cannot do this if we are in the first round after initialize
      // (as previous event meta data is not set properly here)
      if (not m_previousEventMetaData.isEndOfData() and m_eventMetaDataPtr and
          (*m_eventMetaDataPtr != m_previousEventMetaData)) {
        B2FATAL("Two modules setting EventMetaData were discovered: " << m_master->getName() << " and "
                << module->getName());
      }
    } else {
      if (not firstRound) {
        // Also call the event function for the master, but not the first time
        callEvent(module);
      }
      // initialize random number state for the event handling after we have
      // recieved the event information from the master module.
      RandomNumbers::initializeEvent(true);
    }

    if (g_signalReceived != 0) {
      if (g_signalReceived != SIGINT) {
        throw StoppedBySignalException(g_signalReceived);
      } else {
        B2DEBUG(10, "Received a SIGINT in the worker process...");
        return true;
      }
    }

    B2ASSERT("The event meta data must always be valid at this stage!", m_eventMetaDataPtr and m_eventMetaDataPtr.isValid());

    if (m_eventMetaDataPtr->isEndOfData()) {
      // Immediately leave the loop and terminate (true)
      return true;
    }

    if (module == m_master and not firstRound) {
      if (m_eventMetaDataPtr->isEndOfRun()) {
        B2DEBUG(10, "Calling endRun()");
        // call endRun() of all modules (internally uses the previous event meta data) and skip to the next event
        m_processStatisticsPtr->suspendGlobal();
        m_inRun = true;
        processEndRun();
        m_processStatisticsPtr->resumeGlobal();

        // Store the current event meta data for the next round
        m_previousEventMetaData = *m_eventMetaDataPtr;

        // Leave this event, but not the full processing (false)
        return false;
      } else if (m_previousEventMetaData.isEndOfData() or m_previousEventMetaData.isEndOfRun()) {
        // The run has changes (or we never had one), so call beginRun() before going on
        m_processStatisticsPtr->suspendGlobal();
        processBeginRun();
        m_processStatisticsPtr->resumeGlobal();
      }

      const bool runChanged = ((m_eventMetaDataPtr->getExperiment() != m_previousEventMetaData.getExperiment()) or
                               (m_eventMetaDataPtr->getRun() != m_previousEventMetaData.getRun()));
      const bool runChangedWithoutNotice = runChanged and not m_previousEventMetaData.isEndOfData()
                                           and not m_previousEventMetaData.isEndOfRun();
      if (runChangedWithoutNotice) {
        m_processStatisticsPtr->suspendGlobal();

        m_inRun = true;
        processEndRun();
        processBeginRun();

        m_processStatisticsPtr->resumeGlobal();
      }

      // make sure we use the event dependent generator again
      RandomNumbers::useEventDependent();

      // and the correct database
      DBStore::Instance().updateEvent();

      // Store the current event meta data for the next round
      m_previousEventMetaData = *m_eventMetaDataPtr;
    }

    // Check for the module conditions, evaluate them and if one is true, switch to the new path
    if (module->evalCondition()) {
      PathPtr condPath = module->getConditionPath();
      // continue with parent Path after condition path is executed?
      if (module->getAfterConditionPath() == Module::EAfterConditionPath::c_Continue) {
        moduleIter = PathIterator(condPath, moduleIter);
      } else {
        moduleIter = PathIterator(condPath);
      }
    } else {
      moduleIter.next();
    }
  }
  return false;
}

std::pair<unsigned int, unsigned int> HLTEventProcessor::checkChildProcesses()
{
  unsigned int needToRestart = 0;

  // Check for processes, which where there last time but are gone now (so they died)
  for (auto iter = m_processList.begin(); iter != m_processList.end();) {
    const auto& pid = *iter;

    // check the status of this process pid
    int status;
    const int result = waitpid(pid, &status, WNOHANG);
    if (result == -1) {
      if (errno == EINTR) {
        // interrupted, try again next time
        ++iter;
        continue;
      } else {
        B2FATAL("waitpid() failed.");
      }
    } else if (result == 0) {
      // No change, so lets continue with the next worker
      ++iter;
      continue;
    }

    B2ASSERT("Do not understand the result of waitpid()", result == pid);

    // state has changed, which means it is dead!
    const auto exitCode = WEXITSTATUS(status);

    // we only need to restart unexpected deads
    if (exitCode != 0) {
      B2WARNING("A worker process has died unexpected!");
      needToRestart += 1;

      sendTerminatedMessage(pid, true);
    }

    // once a process is gone from the global list, remove them from our own, too.
    iter = m_processList.erase(iter);
  }

  return {m_processList.size(), needToRestart};
}

void HLTEventProcessor::release()
{
  for (auto& socket : m_sockets) {
    socket.release();
  }
  m_parent.reset();
}

bool HLTEventProcessor::forkOut()
{
  fflush(stdout);
  fflush(stderr);

  pid_t pid = fork();

  if (pid > 0) {
    m_processList.push_back(pid);
    return false;
  } else if (pid < 0) {
    B2FATAL("fork() failed: " << strerror(errno));
  } else {
    // Child process
    // Reset some python state: signals, threads, gil in the child
    PyOS_AfterFork();
    // InputController becomes useless in child process
    InputController::resetForChildProcess();
    // die when parent dies
    prctl(PR_SET_PDEATHSIG, SIGHUP);

    ProcHandler::setProcessID(getpid());
    return true;
  }
}

void process(PathPtr startPath, const boost::python::list& outputAddresses, bool restartFailedWorkers = false)
{
  static bool already_executed = false;
  B2ASSERT("Can not run process() on HLT twice per file!", not already_executed);

  auto& environment = Environment::Instance();
  B2ASSERT("HLT processing must happen in multiprocessing mode!", environment.getNumberProcesses() > 0);

  namespace py = boost::python;
  std::vector<std::string> outputAddressesAsString;
  size_t nList = py::len(outputAddresses);
  for (size_t iList = 0; iList < nList; ++iList) {
    outputAddressesAsString.emplace_back(py::extract<std::string>(outputAddresses[iList].attr("__str__")()));
  }

  try {
    LogSystem::Instance().resetMessageCounter();
    DataStore::Instance().reset();
    DataStore::Instance().setInitializeActive(true);

    already_executed = true;

    HLTEventProcessor processor(outputAddressesAsString);
    processor.process(startPath, restartFailedWorkers);

    DBStore::Instance().reset();
  } catch (std::exception& e) {
    B2ERROR("Uncaught exception encountered: " << e.what()); //should show module name
    DataStore::Instance().reset(); // ensure we are executed before ROOT's exit handlers
    throw; //and let python's global handler do the rest
  } catch (...) {
    B2ERROR("Uncaught exception encountered!"); //should show module name
    DataStore::Instance().reset(); // ensure we are executed before ROOT's exit handlers
    throw; //and let python's global handler do the rest
  }
}

BOOST_PYTHON_MODULE(hbasf2)
{
  def("process", &process);
}

REGISTER_PYTHON_MODULE(hbasf2)

/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2015  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Christian Pulvermacher, Martin Ritter      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

//first because of python include
#include <framework/core/Module.h>

#include <framework/core/EventProcessor.h>

#include <framework/core/PathIterator.h>
#include <framework/datastore/DataStore.h>
#include <framework/database/DBStore.h>
#include <framework/logging/Logger.h>
#include <framework/core/Environment.h>
#include <framework/core/DataFlowVisualization.h>
#include <framework/core/RandomNumbers.h>

#ifdef HAS_CALLGRIND
#include <valgrind/callgrind.h>
/** Call a module member function and optionally enable valgrind
 * instrumentation around the function call. We need this to call initialize,
 * beginRun, event, endRun and terminate and need to distuingish between using
 * valgrind or not each time. The easiest way to do this is define this short
 * macro.
 */
#define CALL_MODULE(module,x) \
  if(m_profileModule && m_profileModule==module && RUNNING_ON_VALGRIND){\
    CALLGRIND_START_INSTRUMENTATION;\
    module->x();\
    CALLGRIND_STOP_INSTRUMENTATION;\
  }else{\
    module->x();\
  }
#else
#define CALL_MODULE(module, x) module->x()
#endif

#include <TROOT.h>

#include <signal.h>
#include <unistd.h>
#include <cstring>

using namespace std;
using namespace Belle2;

namespace {
  static int gSignalReceived = 0;
  static void signalHandler(int signal)
  {
    gSignalReceived = signal;

    if (signal == SIGINT) {
      EventProcessor::writeToStdErr("Received Ctrl+C, basf2 will exit safely. (Press Ctrl+\\ (SIGQUIT) to abort immediately - this will break output files.)\n");
    }
  }
}
EventProcessor::StoppedBySignalException::StoppedBySignalException(int signal_):
  runtime_error("Execution stopped by signal " + to_string(signal_) + "!"),
  signal(signal_)
{
}

void EventProcessor::writeToStdErr(const char msg[])
{
  //signal handlers are called asynchronously, making many standard functions (including output) dangerous
  //write() is, however, safe, so we'll use that to write to stderr.

  //strlen() not explicitly in safe list, but doesn't have any error handling routines that might alter global state
  const int len = strlen(msg);

  int rc = write(STDERR_FILENO, msg, len);
  (void) rc; //ignore return value (there's nothing we can do about a failed write)

}


EventProcessor::EventProcessor() : m_master(NULL), m_processStatisticsPtr("", DataStore::c_Persistent),
  m_inRun(false)
{

}


EventProcessor::~EventProcessor()
{

}


void EventProcessor::process(PathPtr startPath, long maxEvent)
{
  //Check whether the number of events was set via command line argument
  unsigned int numEventsArgument = Environment::Instance().getNumberEventsOverride();
  if ((numEventsArgument > 0) && ((maxEvent == 0) || (maxEvent > numEventsArgument))) {
    maxEvent = numEventsArgument;
  }

  //Get list of modules which could be executed during the data processing.
  ModulePtrList moduleList = startPath->buildModulePathList();

  //Find the adress of the module we want to profile
  if (!m_profileModuleName.empty()) {
    for (auto module : moduleList) {
      if (module->getName() == m_profileModuleName) {
        m_profileModule = module.get();
        break;
      }
    }
    if (!m_profileModule)
      B2FATAL("Module profiling was requested via --profile, but no module '" << m_profileModuleName << "' was found!");
  }

  //Initialize modules
  processInitialize(moduleList);

  //do we want to visualize DataStore input/ouput?
  if (Environment::Instance().getVisualizeDataFlow()) {
    DataFlowVisualization v(&DataStore::Instance().getDependencyMap());
    v.visualizePath("dataflow.dot", *startPath);
  }

  //Don't start processing in case of no master module
  if (!m_master) {
    B2ERROR("There is no module that provides event and run numbers (EventMetaData). You must add either the EventInfoSetter or an input module (e.g. RootInput) to the beginning of your path.");
  }

  //Check if errors appeared. If yes, don't start the event processing.
  int numLogError = LogSystem::Instance().getMessageCounter(LogConfig::c_Error);
  if ((numLogError == 0) && m_master) {
    installMainSignalHandlers();
    try {
      processCore(startPath, moduleList, maxEvent); //Do the event processing
    } catch (StoppedBySignalException& e) {
      if (e.signal != SIGINT) {
        // close all open ROOT files, ROOT's exit handler will crash otherwise
        gROOT->GetListOfFiles()->Delete();

        B2FATAL(e.what());
      }
      //in case of SIGINT, we move on to processTerminate() to shut down safely
    } catch (...) {
      if (m_eventMetaDataPtr)
        B2ERROR("Exception occured in exp/run/evt: "
                << m_eventMetaDataPtr->getExperiment() << " / "
                << m_eventMetaDataPtr->getRun() << " / "
                << m_eventMetaDataPtr->getEvent());
      throw;
    }

  } else {
    B2FATAL(numLogError << " ERROR(S) occurred! The processing of events will not be started.");
  }

  //Terminate modules
  processTerminate(moduleList);

  LogSystem::Instance().printErrorSummary();

  if (gSignalReceived == SIGINT) {
    B2ERROR("Processing aborted via SIGINT, terminating. Output files have been closed safely and should be readable.");
    installSignalHandler(SIGINT, SIG_DFL);
    raise(SIGINT);
  }
}


//============================================================================
//                            Protected methods
//============================================================================

void EventProcessor::callEvent(Module* module)
{
  LogSystem& logSystem = LogSystem::Instance();
  const bool collectStats = !Environment::Instance().getNoStats();
  // set up logging
  logSystem.updateModule(&(module->getLogConfig()), module->getName());
  // set up statistics is requested
  if (collectStats) m_processStatisticsPtr->startModule();
  // call module
  CALL_MODULE(module, event);
  // stop timing
  if (collectStats) m_processStatisticsPtr->stopModule(module, ModuleStatistics::c_Event);
  // reset logging
  logSystem.updateModule(NULL);
};

void EventProcessor::processInitialize(const ModulePtrList& modulePathList, bool setEventInfo)
{
  LogSystem& logSystem = LogSystem::Instance();

  m_processStatisticsPtr.registerInDataStore();
  //TODO I might want to overwrite it in initialize (e.g. if read from file)
  //     For parallel processing or subevents, I don't want that, though.
  //     Maybe make this a function argument?
  if (!m_processStatisticsPtr)
    m_processStatisticsPtr.create();
  m_processStatisticsPtr->startGlobal();

  for (const ModulePtr& modPtr : modulePathList) {
    Module* module = modPtr.get();

    if (module->hasUnsetForcedParams()) {
      //error message was printed by module
      continue;
    }

    //Set the module dependent log level
    logSystem.updateModule(&(module->getLogConfig()), module->getName());
    DataStore::Instance().getDependencyMap().setModule(*module);

    //Do initialization
    m_processStatisticsPtr->initModule(module);
    m_processStatisticsPtr->startModule();
    CALL_MODULE(module, initialize);
    m_processStatisticsPtr->stopModule(module, ModuleStatistics::c_Init);

    //Set the global log level
    logSystem.updateModule(NULL);

    //Check whether this is the master module
    if (!m_master && DataStore::Instance().getEntry(m_eventMetaDataPtr) != NULL) {
      B2DEBUG(100, "Found module providing EventMetaData: " << module->getName());
      m_master = module;
      if (setEventInfo) {
        callEvent(module);
        // update Database payloads: we now have valid event meta data unless
        // we don't process any events
        if (m_eventMetaDataPtr) DBStore::Instance().update();
      }
    }

    if (gSignalReceived != 0) {
      throw StoppedBySignalException(gSignalReceived);
    }
  }
  m_processStatisticsPtr->stopGlobal(ModuleStatistics::c_Init);
}

void EventProcessor::installSignalHandler(int sig, void (*fn)(int))
{
  struct sigaction s;
  memset(&s, '\0', sizeof(s));

  s.sa_handler = fn;
  sigemptyset(&s.sa_mask);
  if (sig == SIGCHLD)
    s.sa_flags |= SA_NOCLDSTOP; //don't produce signal when children are stopped

  if (sigaction(sig, &s, nullptr) != 0) {
    B2FATAL("Cannot setup signal handler for signal " << sig);
  }
}

void EventProcessor::installMainSignalHandlers(void (*fn)(int))
{
  if (!fn)
    fn = signalHandler;
  installSignalHandler(SIGINT, fn);
  installSignalHandler(SIGTERM, fn);
  installSignalHandler(SIGQUIT, fn);
}

bool EventProcessor::processEvent(PathIterator moduleIter, bool skipMasterModule)
{
  const bool collectStats = !Environment::Instance().getNoStats();

  while (!moduleIter.isDone()) {
    Module* module = moduleIter.get();

    // run the module ... unless we don't want to
    if (!(skipMasterModule && module == m_master)) {
      callEvent(module);
    }

    //Check for end of data
    if ((m_eventMetaDataPtr && (m_eventMetaDataPtr->isEndOfData())) ||
        ((module == m_master) && !m_eventMetaDataPtr)) {
      if (module != m_master) {
        B2WARNING("Event processing stopped by module '" << module->getName() <<
                  "', which is not in control of event processing (does not provide EventMetaData)");
      }
      return true;
    }

    //Handle EventMetaData changes by master module
    if (module == m_master) {

      //initialize random number state for the event
      RandomNumbers::initializeEvent();

      //Check for a change of the run
      if ((m_eventMetaDataPtr->getExperiment() != m_previousEventMetaData.getExperiment()) ||
          (m_eventMetaDataPtr->getRun() != m_previousEventMetaData.getRun())) {

        if (collectStats)
          m_processStatisticsPtr->suspendGlobal();

        processEndRun();
        processBeginRun(skipMasterModule);

        if (collectStats)
          m_processStatisticsPtr->resumeGlobal();
      }

      m_previousEventMetaData = *m_eventMetaDataPtr;

      //make sure we use the event dependent generator again
      RandomNumbers::useEventDependent();

      DBStore::Instance().updateEvent();

    } else {
      //Check for a second master module. Cannot do this if we skipped the
      //master module as the EventMetaData is probably set before we call this
      //function
      if (!skipMasterModule && m_eventMetaDataPtr &&
          (*m_eventMetaDataPtr != m_previousEventMetaData)) {
        B2FATAL("Two modules setting EventMetaData were discovered: " << m_master->getName() << " and " << module->getName());
      }
    }

    if (gSignalReceived != 0) {
      throw StoppedBySignalException(gSignalReceived);
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

void EventProcessor::processCore(PathPtr startPath, const ModulePtrList& modulePathList, long maxEvent, bool isInputProcess)
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
    endProcess = processEvent(moduleIter, isInputProcess && currEvent == 0);

    //Delete event related data in DataStore
    DataStore::Instance().invalidateData(DataStore::c_Event);

    currEvent++;
    if ((maxEvent > 0) && (currEvent >= maxEvent)) endProcess = true;
    if (collectStats)
      m_processStatisticsPtr->stopGlobal(ModuleStatistics::c_Event);
  } //end event loop

  //End last run
  m_eventMetaDataPtr.create();
  processEndRun();
}


void EventProcessor::processTerminate(const ModulePtrList& modulePathList)
{
  LogSystem& logSystem = LogSystem::Instance();
  ModulePtrList::const_reverse_iterator listIter;
  m_processStatisticsPtr->startGlobal();

  for (listIter = modulePathList.rbegin(); listIter != modulePathList.rend(); ++listIter) {
    Module* module = listIter->get();

    //Set the module dependent log level
    logSystem.updateModule(&(module->getLogConfig()), module->getName());

    //Do termination
    m_processStatisticsPtr->startModule();
    CALL_MODULE(module, terminate);
    m_processStatisticsPtr->stopModule(module, ModuleStatistics::c_Term);

    //Set the global log level
    logSystem.updateModule(NULL);
  }

  m_processStatisticsPtr->stopGlobal(ModuleStatistics::c_Term);
}


void EventProcessor::processBeginRun(bool skipDB)
{
  m_inRun = true;

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
    CALL_MODULE(module, beginRun);
    m_processStatisticsPtr->stopModule(module, ModuleStatistics::c_BeginRun);

    //Set the global log level
    logSystem.updateModule(NULL);
  }

  m_processStatisticsPtr->stopGlobal(ModuleStatistics::c_BeginRun);
}


void EventProcessor::processEndRun()
{
  if (!m_inRun)
    return;
  m_inRun = false;

  LogSystem& logSystem = LogSystem::Instance();
  m_processStatisticsPtr->startGlobal();

  const EventMetaData newEventMetaData = *m_eventMetaDataPtr;
  *m_eventMetaDataPtr = m_previousEventMetaData;

  //initialize random generator for end run
  RandomNumbers::initializeEndRun();

  for (const ModulePtr& modPtr : m_moduleList) {
    Module* module = modPtr.get();

    //Set the module dependent log level
    logSystem.updateModule(&(module->getLogConfig()), module->getName());

    //Do endRun() call
    m_processStatisticsPtr->startModule();
    CALL_MODULE(module, endRun);
    m_processStatisticsPtr->stopModule(module, ModuleStatistics::c_EndRun);

    //Set the global log level
    logSystem.updateModule(NULL);
  }
  *m_eventMetaDataPtr = newEventMetaData;

  m_processStatisticsPtr->stopGlobal(ModuleStatistics::c_EndRun);
}

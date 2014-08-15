/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

//first because of python include
#include <framework/core/Module.h>

#include <framework/core/EventProcessor.h>

#include <framework/core/PathIterator.h>
#include <framework/datastore/DataStore.h>
#include <framework/logging/Logger.h>
#include <framework/core/Environment.h>
#include <framework/core/DataFlowVisualization.h>

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

#include <signal.h>
#include <unistd.h>
#include <cstring>

#include <TRandom.h>

using namespace std;
using namespace Belle2;

void EventProcessor::writeToStdErr(const char msg[])
{
  //signal handlers are called asynchronously, making many standard functions (including output) dangerous
  //write() is, however, safe, so we'll use that to write to stderr.

  //strlen() not explicitly in safe list, but doesn't have any error handling routines that might alter global state
  const int len = strlen(msg);

  int rc = write(STDERR_FILENO, msg, len);
  (void) rc; //ignore return value (there's nothing we can do about a failed write)

}


EventProcessor::EventProcessor() : m_master(NULL), m_mainRNG(NULL), m_processStatisticsPtr("", DataStore::c_Persistent), m_inRun(false)
{

}


EventProcessor::~EventProcessor()
{

}


void EventProcessor::process(PathPtr startPath, long maxEvent)
{
  //Check whether the number of events was set via command line argument
  int numEventsArgument = Environment::Instance().getNumberEventsOverride();
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
  }

  //Initialize modules
  processInitialize(moduleList);

  //do we want to visualize DataStore input/ouput?
  if (Environment::Instance().getVisualizeDataFlow()) {
    DataFlowVisualization v(DataStore::Instance().getModuleInfoMap());
    //single graph for entire steering file
    v.generateModulePlots("dataflow.dot", *startPath, true);

    B2INFO("Data flow diagram created. You can use 'dot dataflow.dot -Tps -o dataflow.ps' to create a PostScript file from it.");
  }

  //Don't start processing in case of no master module
  if (!m_master) {
    B2ERROR("There is no module that provides event and run numbers. You must either add the EventInfoSetter module to your path, or, if using an input module, read EventMetaData objects from file.");
  }

  //Check if errors appeared. If yes, don't start the event processing.
  int numLogError = LogSystem::Instance().getMessageCounter(LogConfig::c_Error);
  if ((numLogError == 0) && m_master) {
    setupSignalHandler();
    processCore(startPath, moduleList, maxEvent); //Do the event processing

  } else {
    B2FATAL(numLogError << " ERROR(S) occurred! The processing of events will not be started.");
  }

  //Terminate modules
  processTerminate(moduleList);

  LogSystem::Instance().printErrorSummary();
}


//============================================================================
//                            Protected methods
//============================================================================
static bool ctrl_c = false;
static void signalHandler(int)
{
  ctrl_c = true;

  EventProcessor::writeToStdErr("Received Ctrl+C, basf2 will exit safely. (Press Ctrl+\\ (SIGQUIT) to abort immediately - this may break output files.)\n");
}

void EventProcessor::processInitialize(const ModulePtrList& modulePathList)
{
  //store main RNG to be able to restore it later
  m_mainRNG = gRandom;

  LogSystem& logSystem = LogSystem::Instance();

  m_processStatisticsPtr.registerInDataStore();
  //TODO I might want to overwrite it in initialize (e.g. if read from file)
  //     For parallel processing or subevents, I don't want that, though.
  //     Maybe make this a function argument?
  if (!m_processStatisticsPtr)
    m_processStatisticsPtr.create();
  m_processStatisticsPtr->startGlobal();

  for (ModulePtrList::const_iterator listIter = modulePathList.begin(); listIter != modulePathList.end(); ++listIter) {
    Module* module = listIter->get();

    if (module->hasUnsetForcedParams()) {
      B2ERROR("The module " << module->getName() << " has unset parameters which have to be set by the user!")
      continue;
    }

    //Set the module dependent log level
    logSystem.setModuleLogConfig(&(module->getLogConfig()), module->getName());
    DataStore::Instance().setModule(module->getName());

    //Do initialization
    m_processStatisticsPtr->initModule(module);
    m_processStatisticsPtr->startModule();
    CALL_MODULE(module, initialize);
    m_processStatisticsPtr->stopModule(module, ModuleStatistics::c_Init);

    //Set the global log level
    logSystem.setModuleLogConfig(NULL);

    //Check whether this is the master module
    if (!m_master && DataStore::Instance().getEntry(m_eventMetaDataPtr) != NULL) {
      B2DEBUG(100, "Found master module " << module->getName());
      m_master = module;
    }
  }
  m_processStatisticsPtr->stopGlobal(ModuleStatistics::c_Init);
}

void EventProcessor::setupSignalHandler()
{
  if (signal(SIGINT, signalHandler) == SIG_ERR) {
    B2FATAL("Cannot setup SIGINT signal handler\n");
  }
}

bool EventProcessor::processEvent(PathIterator moduleIter, EventMetaData* previousEventMetaData)
{
  LogSystem& logSystem = LogSystem::Instance();
  const bool collectStats = !Environment::Instance().getNoStats();

  while (!moduleIter.isDone()) {
    Module* module = moduleIter.get();

    //Set the module dependent log level
    logSystem.setModuleLogConfig(&(module->getLogConfig()), module->getName());

    //Call the event method of the module
    if (collectStats)
      m_processStatisticsPtr->startModule();
    CALL_MODULE(module, event);
    if (collectStats)
      m_processStatisticsPtr->stopModule(module, ModuleStatistics::c_Event);

    //Set the global log level
    logSystem.setModuleLogConfig(NULL);

    //Check for end of data
    if ((m_eventMetaDataPtr && (m_eventMetaDataPtr->isEndOfData())) ||
        ((module == m_master) && !m_eventMetaDataPtr)) {
      if (module != m_master) {
        B2WARNING("Event processing stopped by non-master module " << module->getName());
      }
      return true;
    }

    //Handle event meta data changes of the master module
    if (module == m_master) {

      //Check for a change of the run
      if ((m_eventMetaDataPtr->getExperiment() != previousEventMetaData->getExperiment()) ||
          (m_eventMetaDataPtr->getRun() != previousEventMetaData->getRun())) {

        if (collectStats)
          m_processStatisticsPtr->suspendGlobal();
        //End the previous run
        EventMetaData newEventMetaData = *m_eventMetaDataPtr;
        *m_eventMetaDataPtr = *previousEventMetaData;
        processEndRun();
        *m_eventMetaDataPtr = newEventMetaData;

        //Start a new run
        processBeginRun();

        if (collectStats)
          m_processStatisticsPtr->resumeGlobal();
      }

      *previousEventMetaData = *m_eventMetaDataPtr;

    } else {
      //Check for a second master module
      if (m_eventMetaDataPtr && (*m_eventMetaDataPtr != *previousEventMetaData)) {
        B2FATAL("Two master modules were discovered: " << m_master->getName() << " and " << module->getName());
      }
    }

    if (ctrl_c) {
      return true;
    }

    //Check for a module condition, evaluate it and if it is true switch to a new path
    if (module->evalCondition()) {
      PathPtr condPath = module->getConditionPath();
      if (module->getAfterConditionPath() == Module::EAfterConditionPath::c_Continue) { //continue with parent Path after condition path is executed?
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

void EventProcessor::processCore(PathPtr startPath, const ModulePtrList& modulePathList, long maxEvent)
{
  DataStore::Instance().setInitializeActive(false);
  m_moduleList = modulePathList;
  //Remember the previous event meta data, and identify end of data meta data
  EventMetaData previousEventMetaData;
  previousEventMetaData.setEndOfData(); //invalid start state

  const bool collectStats = !Environment::Instance().getNoStats();

  //Loop over the events
  long currEvent = 0;
  bool endProcess = false;
  while (!endProcess) {
    if (collectStats)
      m_processStatisticsPtr->startGlobal();

    gRandom = m_mainRNG;

    //Loop over the modules in the current path
    PathIterator moduleIter(startPath);
    endProcess = processEvent(moduleIter, &previousEventMetaData);

    //Delete event related data in DataStore
    DataStore::Instance().invalidateData(DataStore::c_Event);

    currEvent++;
    if ((maxEvent > 0) && (currEvent >= maxEvent)) endProcess = true;
    if (collectStats)
      m_processStatisticsPtr->stopGlobal(ModuleStatistics::c_Event);
  } //end event loop

  //End last run
  m_eventMetaDataPtr.create();
  *m_eventMetaDataPtr = previousEventMetaData;
  processEndRun();
}


void EventProcessor::processTerminate(const ModulePtrList& modulePathList)
{
  gRandom = m_mainRNG;

  LogSystem& logSystem = LogSystem::Instance();
  ModulePtrList::const_reverse_iterator listIter;
  m_processStatisticsPtr->startGlobal();

  for (listIter = modulePathList.rbegin(); listIter != modulePathList.rend(); ++listIter) {
    Module* module = listIter->get();

    //Set the module dependent log level
    logSystem.setModuleLogConfig(&(module->getLogConfig()), module->getName());

    //Do termination
    m_processStatisticsPtr->startModule();
    CALL_MODULE(module, terminate);
    m_processStatisticsPtr->stopModule(module, ModuleStatistics::c_Term);

    //Set the global log level
    logSystem.setModuleLogConfig(NULL);
  }

  m_processStatisticsPtr->stopGlobal(ModuleStatistics::c_Term);
}


void EventProcessor::processBeginRun()
{
  m_inRun = true;
  gRandom = m_mainRNG;

  LogSystem& logSystem = LogSystem::Instance();
  ModulePtrList::const_iterator listIter;
  m_processStatisticsPtr->startGlobal();

  for (listIter = m_moduleList.begin(); listIter != m_moduleList.end(); ++listIter) {
    Module* module = listIter->get();

    //Set the module dependent log level
    logSystem.setModuleLogConfig(&(module->getLogConfig()), module->getName());

    //Do beginRun() call
    m_processStatisticsPtr->startModule();
    CALL_MODULE(module, beginRun);
    m_processStatisticsPtr->stopModule(module, ModuleStatistics::c_BeginRun);

    //Set the global log level
    logSystem.setModuleLogConfig(NULL);
  }
  gRandom = m_mainRNG;

  m_processStatisticsPtr->stopGlobal(ModuleStatistics::c_BeginRun);
}


void EventProcessor::processEndRun()
{
  if (!m_inRun)
    return;
  m_inRun = false;

  gRandom = m_mainRNG;

  LogSystem& logSystem = LogSystem::Instance();
  ModulePtrList::const_iterator listIter;
  m_processStatisticsPtr->startGlobal();

  for (listIter = m_moduleList.begin(); listIter != m_moduleList.end(); ++listIter) {
    Module* module = listIter->get();

    //Set the module dependent log level
    logSystem.setModuleLogConfig(&(module->getLogConfig()), module->getName());

    //Do endRun() call
    m_processStatisticsPtr->startModule();
    CALL_MODULE(module, endRun);
    m_processStatisticsPtr->stopModule(module, ModuleStatistics::c_EndRun);

    //Set the global log level
    logSystem.setModuleLogConfig(NULL);
  }
  gRandom = m_mainRNG;

  m_processStatisticsPtr->stopGlobal(ModuleStatistics::c_EndRun);
}

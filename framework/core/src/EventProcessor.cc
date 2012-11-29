/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/core/EventProcessor.h>

#include <framework/core/ModuleManager.h>
#include <framework/core/ModuleStatistics.h>
#include <framework/core/PathManager.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/logging/Logger.h>
#include <framework/core/Environment.h>
#include <framework/core/DataFlowVisualization.h>

#ifdef HAS_CALLGRIND
#include <valgrind/callgrind.h>
#endif

#include <signal.h>
#include <unistd.h>

using namespace std;
using namespace Belle2;


EventProcessor::EventProcessor(PathManager& pathManager) : m_pathManager(pathManager), m_master(0)
{

}


EventProcessor::~EventProcessor()
{

}


void EventProcessor::process(PathPtr startPath, long maxEvent)
{
#ifdef HAS_CALLGRIND
  CALLGRIND_START_INSTRUMENTATION;
#endif
  //Check whether the number of events was set via command line argument
  int numEventsArgument = Environment::Instance().getNumberEventsOverride();
  if ((numEventsArgument > 0) && ((maxEvent == 0) || (maxEvent > numEventsArgument))) {
    maxEvent = numEventsArgument;
  }

  //Get list of modules which could be executed during the data processing.
  ModulePtrList moduleList = m_pathManager.buildModulePathList(startPath);

  //Initialize modules
  processInitialize(moduleList);

  //Don't start processing in case of no master module
  if (!m_master) {
    B2ERROR("There is no module that provides event and run numbers. You must either add the EvtMetaGen module to your path, or, if using an input module, read EventMetaData objects from file.");
  }

  //Check if errors appeared. If yes, don't start the event processing.
  int numLogError = LogSystem::Instance().getMessageCounter(LogConfig::c_Error);
  if ((numLogError == 0) && m_master) {
    processCore(startPath, moduleList, maxEvent); //Do the event processing

  } else {
    B2ERROR(numLogError << " ERROR(S) occurred! The processing of events will not be started.");
  }

  //Terminate modules
  processTerminate(moduleList);
#ifdef HAS_CALLGRIND
  CALLGRIND_STOP_INSTRUMENTATION;
#endif

  //print summary on warnings/errors
  int numLogWarn = LogSystem::Instance().getMessageCounter(LogConfig::c_Warning);
  numLogError = LogSystem::Instance().getMessageCounter(LogConfig::c_Error);
  LogConfig& logConfig = *LogSystem::Instance().getLogConfig();
  // only show level & message
  logConfig.setLogInfo(LogConfig::c_Warning, LogConfig::c_Level | LogConfig::c_Message);
  logConfig.setLogInfo(LogConfig::c_Error, LogConfig::c_Level | LogConfig::c_Message);
  if (numLogWarn)
    B2WARNING(numLogWarn << " warnings occured.");
  if (numLogError)
    B2ERROR(numLogError << " errors occured.");
}


//============================================================================
//                            Protected methods
//============================================================================
static bool ctrl_c = false;
static void signalHandler(int)
{
  ctrl_c = true;

  //signal handlers are called asynchronously, making many standard functions (including output) dangerous
  //write() is, however, safe, so we'll use that to write to stderr.
  const char msg[] = "Received Ctrl+C, basf2 will exit safely. (Press Ctrl+\\ (SIGQUIT) to abort immediately - this may break output files.)\n";
  const int len = sizeof(msg) / sizeof(char) - 1; //minus NULL byte

  write(STDERR_FILENO, msg, len);
}

void EventProcessor::processInitialize(const ModulePtrList& modulePathList)
{
#ifdef HAS_CALLGRIND
  CALLGRIND_ZERO_STATS;
#endif
  LogSystem& logSystem = LogSystem::Instance();
  ModulePtrList::const_iterator listIter;
  ModuleStatistics& stats = ModuleStatistics::getInstance();
  stats.startGlobal();
  DataStore::Instance().setInitializeActive(true);

  for (listIter = modulePathList.begin(); listIter != modulePathList.end(); ++listIter) {
    Module* module = listIter->get();

    if (module->hasUnsetForcedParams()) {
      B2ERROR("The module " << module->getName() << " has unset parameters which have to be set by the user!")
      continue;
    }

    //Set the module dependent log level
    logSystem.setModuleLogConfig(&(module->getLogConfig()), module->getName());
    DataStore::Instance().setModule(module->getName());

    //Do initialization
    stats.startModule();
    module->initialize();
    stats.stopModule(*module, ModuleStatistics::c_Init);

    //Set the global log level
    logSystem.setModuleLogConfig(NULL);

    //Check whether this is the master module
    if (!m_master && DataStore::Instance().hasEntry(DataStore::objectName<EventMetaData>(""), DataStore::c_Event, EventMetaData::Class(), false)) {
      B2DEBUG(100, "Found master module " << module->getName());
      m_master = module;
    }
  }
  DataStore::Instance().setInitializeActive(false);
  stats.stopGlobal(ModuleStatistics::c_Init);
#ifdef HAS_CALLGRIND
  CALLGRIND_DUMP_STATS_AT("initialize");
#endif

  //do we want to visualize DataStore input/ouput?
  if (Environment::Instance().getVisualizeDataFlow()) {
    DataFlowVisualization v(DataStore::Instance().getModuleInfoMap(), modulePathList);
    //generate graphs for each module
    v.generateModulePlots("dataflow.dot");

    //single graph for entire steering file
    v.generateModulePlots("dataflow_all.dot", true);

    B2INFO("Data flow diagrams created. You can use 'dot dataflow.dot -Tps -o dataflow.ps' to create a PostScript file from them.");
  }
}


void EventProcessor::processCore(PathPtr startPath, const ModulePtrList& modulePathList, long maxEvent)
{
  if (signal(SIGINT, signalHandler) == SIG_ERR) {
    B2FATAL("Cannot setup SIGINT signal handler\n");
  }
#ifdef HAS_CALLGRIND
  CALLGRIND_ZERO_STATS;
#endif
  long currEvent = 0;
  bool endProcess = false;
  PathPtr currPath;
  LogSystem& logSystem = LogSystem::Instance();

  //Remember the previous event meta data, and identify end of data meta data
  EventMetaData previousEventMetaData;
  EventMetaData endEventMetaData;
  endEventMetaData.setEndOfData();
  StoreObjPtr<EventMetaData> eventMetaDataPtr;

  ModuleStatistics& stats = ModuleStatistics::getInstance();

  //Loop over the events
  while (!endProcess) {
    stats.startGlobal();

    //Loop over the modules in the current path
    currPath = startPath;
    ModulePtrList modules = currPath->getModules();
    ModulePtrList::const_iterator moduleIter = modules.begin();
    while (!endProcess and moduleIter != modules.end()) {
      Module* module = moduleIter->get();

      //Set the module dependent log level
      logSystem.setModuleLogConfig(&(module->getLogConfig()), module->getName());

      //Call the event method of the module
      stats.startModule();
      module->event();
      stats.stopModule(*module, ModuleStatistics::c_Event);

      //Set the global log level
      logSystem.setModuleLogConfig(NULL);

      //Check for end of data
      if ((eventMetaDataPtr && (*eventMetaDataPtr == endEventMetaData)) ||
          ((module == m_master) && !eventMetaDataPtr)) {
        if (module != m_master) {
          B2WARNING("Event processing stopped by non-master module " << module->getName());
        }
        endProcess = true;
        break;
      }

      //Handle event meta data changes of the master module
      if (module == m_master) {

        //Check for a change of the run
        if ((eventMetaDataPtr->getExperiment() != previousEventMetaData.getExperiment()) ||
            (eventMetaDataPtr->getRun() != previousEventMetaData.getRun())) {

          stats.stopGlobal(ModuleStatistics::c_Event, true);
          //End the previous run
          if (currEvent > 0) {
#ifdef HAS_CALLGRIND
            CALLGRIND_DUMP_STATS_AT("event");
#endif
            EventMetaData newEventMetaData = *eventMetaDataPtr;
            *eventMetaDataPtr = previousEventMetaData;
            processEndRun(modulePathList);
            *eventMetaDataPtr = newEventMetaData;
          }

          //Start a new run
          processBeginRun(modulePathList);

#ifdef HAS_CALLGRIND
          CALLGRIND_ZERO_STATS;
#endif
          stats.startGlobal();
        }

        previousEventMetaData = *eventMetaDataPtr;

      } else {
        //Check for a second master module
        if (eventMetaDataPtr && (*eventMetaDataPtr != previousEventMetaData)) {
          B2FATAL("Two master modules were discovered: " << m_master->getName()
                  << " and " << module->getName());
        }
      }

      if (ctrl_c) {
        endProcess = true;
      }
      if (!endProcess) {
        //Check for a module condition, evaluate it and if it is true switch to a new path
        if (module->evalCondition()) {
          currPath = module->getConditionPath();
          modules = currPath->getModules();
          moduleIter = modules.begin();
        } else {
          ++moduleIter;
        }
      }
    }

    //Delete event related data in DataStore
    DataStore::Instance().clearMaps(DataStore::c_Event);

    currEvent++;
    if ((maxEvent > 0) && (currEvent >= maxEvent)) endProcess = true;
    stats.stopGlobal(ModuleStatistics::c_Event);
  }
#ifdef HAS_CALLGRIND
  CALLGRIND_DUMP_STATS_AT("event");
#endif

  //End last run
  if (currEvent > 0) {
    eventMetaDataPtr.create();
    *eventMetaDataPtr = previousEventMetaData;
    processEndRun(modulePathList);
  }
}


void EventProcessor::processTerminate(const ModulePtrList& modulePathList)
{
#ifdef HAS_CALLGRIND
  CALLGRIND_ZERO_STATS;
#endif
  LogSystem& logSystem = LogSystem::Instance();
  ModulePtrList::const_reverse_iterator listIter;
  ModuleStatistics& stats = ModuleStatistics::getInstance();
  stats.startGlobal();

  for (listIter = modulePathList.rbegin(); listIter != modulePathList.rend(); ++listIter) {
    Module* module = listIter->get();

    //Set the module dependent log level
    logSystem.setModuleLogConfig(&(module->getLogConfig()), module->getName());

    //Do termination
    stats.startModule();
    module->terminate();
    stats.stopModule(*module, ModuleStatistics::c_Term);

    //Set the global log level
    logSystem.setModuleLogConfig(NULL);
  }

  //Delete persistent data in DataStore
  DataStore::Instance().clearMaps(DataStore::c_Persistent);
  stats.stopGlobal(ModuleStatistics::c_Term);
#ifdef HAS_CALLGRIND
  CALLGRIND_DUMP_STATS_AT("terminate");
#endif
}


void EventProcessor::processBeginRun(const ModulePtrList& modulePathList)
{
#ifdef HAS_CALLGRIND
  CALLGRIND_ZERO_STATS;
#endif
  LogSystem& logSystem = LogSystem::Instance();
  ModulePtrList::const_iterator listIter;
  ModuleStatistics& stats = ModuleStatistics::getInstance();
  stats.startGlobal();

  for (listIter = modulePathList.begin(); listIter != modulePathList.end(); ++listIter) {
    Module* module = listIter->get();

    //Set the module dependent log level
    logSystem.setModuleLogConfig(&(module->getLogConfig()), module->getName());

    //Do beginRun() call
    stats.startModule();
    module->beginRun();
    stats.stopModule(*module, ModuleStatistics::c_BeginRun);

    //Set the global log level
    logSystem.setModuleLogConfig(NULL);
  }
  stats.stopGlobal(ModuleStatistics::c_BeginRun);
#ifdef HAS_CALLGRIND
  CALLGRIND_DUMP_STATS_AT("beginRun");
#endif

}


void EventProcessor::processEndRun(const ModulePtrList& modulePathList)
{
#ifdef HAS_CALLGRIND
  CALLGRIND_ZERO_STATS;
#endif
  LogSystem& logSystem = LogSystem::Instance();
  ModulePtrList::const_iterator listIter;
  ModuleStatistics& stats = ModuleStatistics::getInstance();
  stats.startGlobal();

  for (listIter = modulePathList.begin(); listIter != modulePathList.end(); ++listIter) {
    Module* module = listIter->get();

    //Set the module dependent log level
    logSystem.setModuleLogConfig(&(module->getLogConfig()), module->getName());

    //Do endRun() call
    stats.startModule();
    module->endRun();
    stats.stopModule(*module, ModuleStatistics::c_EndRun);

    //Set the global log level
    logSystem.setModuleLogConfig(NULL);
  }

  stats.stopGlobal(ModuleStatistics::c_EndRun);
#ifdef HAS_CALLGRIND
  CALLGRIND_DUMP_STATS_AT("endRun");
#endif
}

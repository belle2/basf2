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
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/logging/Logger.h>

#ifdef HAS_CALLGRIND
#include <valgrind/callgrind.h>
#endif

using namespace std;
using namespace Belle2;


EventProcessor::EventProcessor(PathManager& pathManager) : m_pathManager(pathManager)
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
  //Get list of modules which could be executed during the data processing.
  ModulePtrList moduleList = m_pathManager.buildModulePathList(startPath);

  //Initialize modules
  processInitialize(moduleList);

  //Check if errors appeared. If yes, don't start the event processing.
  int numLogError = LogSystem::Instance().getMessageCounter(LogConfig::c_Error);
  if (numLogError == 0) {
    processCore(startPath, moduleList, maxEvent); //Do the event processing

  } else {
    B2ERROR(numLogError << " ERROR(S) occurred ! The processing of events will not be started.");
  }

  //Terminate modules
  processTerminate(moduleList);
#ifdef HAS_CALLGRIND
  CALLGRIND_STOP_INSTRUMENTATION;
#endif
}


//============================================================================
//                            Protected methods
//============================================================================

void EventProcessor::processInitialize(const ModulePtrList& modulePathList)
{
#ifdef HAS_CALLGRIND
  CALLGRIND_ZERO_STATS;
#endif
  LogSystem& logSystem = LogSystem::Instance();
  ModulePtrList::const_iterator listIter;
  ModuleStatistics &stats = ModuleStatistics::getInstance();
  stats.startGlobal(ModuleStatistics::c_Init);
  DataStore::Instance().setInitializeActive(true);

  for (listIter = modulePathList.begin(); listIter != modulePathList.end(); listIter++) {
    Module* module = listIter->get();

    if (module->hasUnsetForcedParams()) {
      B2ERROR("The module " << module->getName() << " has unset parameters which have to be set by the user !")
      continue;
    }

    //Set the module dependent log level
    logSystem.setModuleLogConfig(&(module->getLogConfig()), module->getName());

    //Do initialization
    stats.startModule(*module, ModuleStatistics::c_Init);
    module->initialize();
    stats.stopModule(*module, ModuleStatistics::c_Init);

    //Set the global log level
    logSystem.setModuleLogConfig(NULL);
  }
  DataStore::Instance().setInitializeActive(false);
  stats.stopGlobal(ModuleStatistics::c_Init);
#ifdef HAS_CALLGRIND
  CALLGRIND_DUMP_STATS_AT("initialize");
#endif
}


void EventProcessor::processCore(PathPtr startPath, const ModulePtrList& modulePathList, long maxEvent)
{
#ifdef HAS_CALLGRIND
  CALLGRIND_ZERO_STATS;
#endif
  long currEvent = 0;
  bool endProcess = false;
  PathPtr currPath;
  ModulePtrList::const_iterator moduleIter;
  LogSystem& logSystem = LogSystem::Instance();

  //Remember the previous event meta data, and identify end of data meta data
  EventMetaData previousEventMetaData;
  EventMetaData noEventMetaData;
  EventMetaData endEventMetaData;
  endEventMetaData.setEndOfData();

  //Pointer to master module;
  Module* master = 0;

  ModuleStatistics &stats = ModuleStatistics::getInstance();

  //Loop over the events
  while (!endProcess) {
    stats.startGlobal(ModuleStatistics::c_Event);

    //Loop over the modules in the current path
    currPath = startPath;
    moduleIter = currPath->getModules().begin();
    while ((!endProcess) && (moduleIter != currPath->getModules().end())) {
      Module* module = moduleIter->get();

      //Set the module dependent log level
      logSystem.setModuleLogConfig(&(module->getLogConfig()), module->getName());

      //Call the event method of the module
      stats.startModule(*module, ModuleStatistics::c_Event);
      module->event();
      stats.stopModule(*module, ModuleStatistics::c_Event);

      //Set the global log level
      logSystem.setModuleLogConfig(NULL);

      //Determine the master module
      StoreObjPtr<EventMetaData> eventMetaDataPtr("EventMetaData", DataStore::c_Event);
      if (!master && (currEvent == 0)) {
        if (*eventMetaDataPtr != previousEventMetaData) {
          master = module;
        }
      }

      //Check for end of data
      if ((*eventMetaDataPtr == endEventMetaData) ||
          ((module == master) && (*eventMetaDataPtr == noEventMetaData))) {
        if (module == master) {
          B2WARNING("Event processing stopped by non-master module " << module->getName());
        }
        endProcess = true;
        break;
      }

      //Handle event meta data changes of the master module
      if (module == master) {

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
          stats.startGlobal(ModuleStatistics::c_Event);
        }

        previousEventMetaData = *eventMetaDataPtr;

      } else {

        //Check for a second master module
        if ((*eventMetaDataPtr != noEventMetaData) &&
            (*eventMetaDataPtr != previousEventMetaData)) {
          B2FATAL("Two master modules were discovered: " << master->getName()
                  << " and " << module->getName());
        }
      }

      if (!endProcess) {
        //Check for a module condition, evaluate it and if it is true switch to a new path
        if (module->evalCondition()) {
          currPath = module->getConditionPath();
          moduleIter = currPath->getModules().begin();
        } else moduleIter++;
      }
    }

    //Stop processing in case of no master module
    if (!master) {
      B2WARNING("There is no module that provides event and run numbers. Stop processing");
      endProcess = true;
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
  if (master && (currEvent > 0)) {
    StoreObjPtr<EventMetaData> eventMetaDataPtr("EventMetaData", DataStore::c_Event);
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
  ModuleStatistics &stats = ModuleStatistics::getInstance();
  stats.startGlobal(ModuleStatistics::c_Term);

  for (listIter = modulePathList.rbegin(); listIter != modulePathList.rend(); listIter++) {
    Module* module = listIter->get();

    //Set the module dependent log level
    logSystem.setModuleLogConfig(&(module->getLogConfig()), module->getName());

    //Do termination
    stats.startModule(*module, ModuleStatistics::c_Term);
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
  ModuleStatistics &stats = ModuleStatistics::getInstance();
  stats.startGlobal(ModuleStatistics::c_BeginRun);

  for (listIter = modulePathList.begin(); listIter != modulePathList.end(); listIter++) {
    Module* module = listIter->get();

    //Set the module dependent log level
    logSystem.setModuleLogConfig(&(module->getLogConfig()), module->getName());

    //Do beginRun() call
    stats.startModule(*module, ModuleStatistics::c_BeginRun);
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
  ModuleStatistics &stats = ModuleStatistics::getInstance();
  stats.startGlobal(ModuleStatistics::c_EndRun);

  for (listIter = modulePathList.begin(); listIter != modulePathList.end(); listIter++) {
    Module* module = listIter->get();

    //Set the module dependent log level
    logSystem.setModuleLogConfig(&(module->getLogConfig()), module->getName());

    //Do endRun() call
    stats.startModule(*module, ModuleStatistics::c_EndRun);
    module->endRun();
    stats.stopModule(*module, ModuleStatistics::c_EndRun);

    //Set the global log level
    logSystem.setModuleLogConfig(NULL);
  }

  //Delete run related data in DataStore
  DataStore::Instance().clearMaps(DataStore::c_Run);
  stats.stopGlobal(ModuleStatistics::c_EndRun);
#ifdef HAS_CALLGRIND
  CALLGRIND_DUMP_STATS_AT("endRun");
#endif
}

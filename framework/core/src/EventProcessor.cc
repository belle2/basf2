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
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/EventMetaData.h>
#include <framework/logging/Logger.h>

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
}


//============================================================================
//                            Protected methods
//============================================================================

void EventProcessor::processInitialize(const ModulePtrList& modulePathList)
{
  LogSystem& logSystem = LogSystem::Instance();
  ModulePtrList::const_iterator listIter;

  for (listIter = modulePathList.begin(); listIter != modulePathList.end(); listIter++) {
    Module* module = listIter->get();

    if (module->hasUnsetForcedParams()) {
      B2ERROR("The module " << module->getName() << " has unset parameters which have to be set by the user !")
      continue;
    }

    //Set the module dependent log level
    logSystem.setModuleLogConfig(&(module->getLogConfig()));

    //Do initialization
    module->initialize();

    //Set the global log level
    logSystem.setModuleLogConfig(NULL);
  }
}


void EventProcessor::processCore(PathPtr startPath, const ModulePtrList& modulePathList, long maxEvent)
{
  long currEvent = 0;
  bool endProcess = false;
  PathPtr currPath;
  ModulePtrList::const_iterator moduleIter;
  LogSystem& logSystem = LogSystem::Instance();

  //Remember the previous event meta data, and identify end of data meta data
  EventMetaData previousEventMetaData;
  EventMetaData endEventMetaData;

  //Pointer to master module;
  Module* master = 0;

  //Loop over the events
  while (!endProcess) {

    //Loop over the modules in the current path
    currPath = startPath;
    moduleIter = currPath->getModules().begin();
    while ((!endProcess) && (moduleIter != currPath->getModules().end())) {
      Module* module = moduleIter->get();

      //Set the module dependent log level
      logSystem.setModuleLogConfig(&(module->getLogConfig()));

      //Call the event method of the module
      module->event();

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
      if (master && (*eventMetaDataPtr == endEventMetaData)) {
        if (module != master) {
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

          //End the previous run
          if (currEvent > 0) {
            EventMetaData newEventMetaData = *eventMetaDataPtr;
            *eventMetaDataPtr = previousEventMetaData;
            processEndRun(modulePathList);
            *eventMetaDataPtr = newEventMetaData;
          }

          //Start a new run
          processBeginRun(modulePathList);
        }

        previousEventMetaData = *eventMetaDataPtr;

      } else {

        //Check for a second master module
        if (*eventMetaDataPtr != previousEventMetaData) {
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
  }

  //End last run
  if (master && (currEvent > 0)) {
    StoreObjPtr<EventMetaData> eventMetaDataPtr("EventMetaData", DataStore::c_Event);
    *eventMetaDataPtr = previousEventMetaData;
    processEndRun(modulePathList);
  }
}


void EventProcessor::processTerminate(const ModulePtrList& modulePathList)
{
  LogSystem& logSystem = LogSystem::Instance();
  ModulePtrList::const_reverse_iterator listIter;

  for (listIter = modulePathList.rbegin(); listIter != modulePathList.rend(); listIter++) {
    Module* module = listIter->get();

    //Set the module dependent log level
    logSystem.setModuleLogConfig(&(module->getLogConfig()));

    //Do termination
    module->terminate();

    //Set the global log level
    logSystem.setModuleLogConfig(NULL);
  }

  //Delete persistent data in DataStore
  DataStore::Instance().clearMaps(DataStore::c_Persistent);
}


void EventProcessor::processBeginRun(const ModulePtrList& modulePathList)
{
  LogSystem& logSystem = LogSystem::Instance();
  ModulePtrList::const_iterator listIter;

  for (listIter = modulePathList.begin(); listIter != modulePathList.end(); listIter++) {
    Module* module = listIter->get();

    //Set the module dependent log level
    logSystem.setModuleLogConfig(&(module->getLogConfig()));

    //Do beginRun() call
    module->beginRun();

    //Set the global log level
    logSystem.setModuleLogConfig(NULL);
  }
}


void EventProcessor::processEndRun(const ModulePtrList& modulePathList)
{
  LogSystem& logSystem = LogSystem::Instance();
  ModulePtrList::const_iterator listIter;

  for (listIter = modulePathList.begin(); listIter != modulePathList.end(); listIter++) {
    Module* module = listIter->get();

    //Set the module dependent log level
    logSystem.setModuleLogConfig(&(module->getLogConfig()));

    //Do endRun() call
    module->endRun();

    //Set the global log level
    logSystem.setModuleLogConfig(NULL);
  }

  //Delete run related data in DataStore
  DataStore::Instance().clearMaps(DataStore::c_Run);
}

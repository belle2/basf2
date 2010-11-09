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
#include <framework/datastore/StoreDefs.h>
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


void EventProcessor::process(PathPtr startPath, long maxEvent, long runNumber)
{
  ModuleManager& moduleManager = ModuleManager::Instance();

  //Get list of modules which could be executed during the data processing.
  ModulePtrList moduleList = m_pathManager.buildModulePathList(startPath);

  //Check if there is at least one module in the chain, which can specify the end of the data flow (if maxEvent is <= 0).
  if (maxEvent <= 0) {
    ModulePtrList selEoDModuleList = moduleManager.getModulesByProperties(moduleList, Module::c_TriggersEndOfData);
    if (selEoDModuleList.size() == 0) {
      B2ERROR("There must be at least one module in the chain which can specify the end of the data flow.")
      return;
    }
  }

  //Check if there is exactly one module in the chain, which can specify the begin of a new run (if runNumber < 0).
  if (runNumber < 0) {
    ModulePtrList selBnRModuleList = moduleManager.getModulesByProperties(moduleList, Module::c_TriggersNewRun);
    if (selBnRModuleList.size() != 1) {
      B2ERROR("There are currently " << selBnRModuleList.size() << " modules in the chain which specify the beginning of a new run. There is exactly one module of this type allowed.")
      return;
    }
  } else {
    //Store the run number in the MetaData object
    StoreObjPtr<EventMetaData> eventMetaDataPtr("EventMetaData", c_Persistent);
    eventMetaDataPtr->setRun(runNumber);
  }

  //Initialize modules
  processInitialize(moduleList);

  //Check if errors appeared. If yes, don't start the event processing.
  int numLogError = LogSystem::Instance().getMessageCounter(LogConfig::c_Error);
  if (numLogError == 0) {
    if (runNumber > -1) processBeginRun(moduleList); //If the run number was set, start a new run manually
    processCore(startPath, moduleList, maxEvent); //Do the event processing

  } else {
    B2ERROR(numLogError << " B2ERROR(S) occurred ! The processing of events will not be started.");
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
    logSystem.setModuleLogConfig(module->config());

    //Do initialization
    module->initialize();

    //Set the global log level
    logSystem.setModuleLogConfig(0);
  }
}


void EventProcessor::processCore(PathPtr startPath, const ModulePtrList& modulePathList, long maxEvent)
{
  long currEvent = 0;
  bool normalEvent = true;
  bool endProcess = false;
  PathPtr currPath;
  ModulePtrList::const_iterator moduleIter;
  LogSystem& logSystem = LogSystem::Instance();

  //Loop over the events
  while (!endProcess) {

    //Loop over the modules in the current path
    currPath = startPath;
    moduleIter = currPath->getModules().begin();
    while ((!endProcess) && (moduleIter != currPath->getModules().end())) {
      Module* module = moduleIter->get();
      normalEvent = true;

      //Set the module dependent log level
      logSystem.setModuleLogConfig(module->config());

      //Call the event method of the module
      module->event();

      //Set the global log level
      logSystem.setModuleLogConfig(0);

      //Check the returned process record type
      switch (module->getProcessRecordType()) {
        case Module::prt_Event     :
          break;

        case Module::prt_BeginRun  :
          //If the current module is allowed to trigger a new run, call the beginRun() methods of all modules
          //and continue with the first module in the startPath.
          if (module->hasProperties(Module::c_TriggersNewRun)) {
            processBeginRun(modulePathList);
            currPath = startPath;
            moduleIter = currPath->getModules().begin();
            normalEvent = false;
          } else B2WARNING("Module " << module->getName() << "requested to start a new run, but doesn't have the necessary property set. Request was ignored.");
          break;

        case Module::prt_EndRun    :
          //If the current module is allowed to trigger a new run, call the endRun() methods of all modules
          //and continue with the first module in the startPath.
          if (module->hasProperties(Module::c_TriggersNewRun)) {
            processEndRun(modulePathList);
            currPath = startPath;
            moduleIter = currPath->getModules().begin();
            normalEvent = false;
          } else B2WARNING("Module " << module->getName() << "requested to end a run, but doesn't have the necessary property set. Request was ignored.");
          break;

        case Module::prt_EndOfData :
          //If the current module is allowed to end the process and the end of data flow is reached, stop the event process.
          if (module->hasProperties(Module::c_TriggersEndOfData)) {
            endProcess = true;
          } else B2WARNING("Module " << module->getName() << "requested to stop the event processing (end of data), but doesn't have the necessary property set. Request was ignored.");
          break;
      }

      if ((normalEvent) && (!endProcess)) {
        //Check for a module condition, evaluate it and if it is true switch to a new path
        if (module->evalCondition()) {
          currPath = module->getConditionPath();
          moduleIter = currPath->getModules().begin();
        } else moduleIter++;
      }
    }

    //Delete event related data in DataStore
    DataStore::Instance().clearMaps(c_Event);

    currEvent++;
    if ((maxEvent > 0) && (currEvent >= maxEvent)) endProcess = true;
  }
}


void EventProcessor::processTerminate(const ModulePtrList& modulePathList)
{
  LogSystem& logSystem = LogSystem::Instance();
  ModulePtrList::const_reverse_iterator listIter;

  for (listIter = modulePathList.rbegin(); listIter != modulePathList.rend(); listIter++) {
    Module* module = listIter->get();

    //Set the module dependent log level
    logSystem.setModuleLogConfig(module->config());

    //Do termination
    module->terminate();

    //Set the global log level
    logSystem.setModuleLogConfig(0);
  }

  //Delete persistent data in DataStore
  DataStore::Instance().clearMaps(c_Persistent);
}


void EventProcessor::processBeginRun(const ModulePtrList& modulePathList)
{
  LogSystem& logSystem = LogSystem::Instance();
  ModulePtrList::const_iterator listIter;

  for (listIter = modulePathList.begin(); listIter != modulePathList.end(); listIter++) {
    Module* module = listIter->get();

    //Set the module dependent log level
    logSystem.setModuleLogConfig(module->config());

    //Do beginRun() call
    module->beginRun();

    //Set the global log level
    logSystem.setModuleLogConfig(0);
  }
}


void EventProcessor::processEndRun(const ModulePtrList& modulePathList)
{
  LogSystem& logSystem = LogSystem::Instance();
  ModulePtrList::const_iterator listIter;

  for (listIter = modulePathList.begin(); listIter != modulePathList.end(); listIter++) {
    Module* module = listIter->get();

    //Set the module dependent log level
    logSystem.setModuleLogConfig(module->config());

    //Do endRun() call
    module->endRun();

    //Set the global log level
    logSystem.setModuleLogConfig(0);
  }

  //Delete run related data in DataStore
  DataStore::Instance().clearMaps(c_Run);
}

/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/core/ModuleChain.h>
#include <framework/core/ModuleManager.h>
#include <framework/logging/LogSystem.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreDefs.h>

using namespace std;
using namespace Belle2;


ModuleChain::ModuleChain()
{
  m_moduleList = new ModuleList();
  m_pathList = new PathList();
}


ModuleChain::~ModuleChain()
{
  delete m_pathList;
  delete m_moduleList;
}


ModulePtr ModuleChain::registerModule(const string& type) throw(FwExcModuleNotRegistered)
{
  try {
    return m_moduleList->createModule(type);
  } catch (FwExcModuleNotCreated& exc) {
    throw FwExcModuleNotRegistered(type);
  }
}


PathPtr ModuleChain::addPath() throw(FwExcPathNotAdded)
{
  try {
    return m_pathList->createPath();
  } catch (FwExcPathNotCreated& exc) {
    throw FwExcPathNotAdded();
  }
}


ModulePtrList ModuleChain::getModulePathList(PathPtr startPath) const
{
  return m_pathList->buildModulePathList(startPath);
}


ModulePtrList ModuleChain::getModulesByProperties(const ModulePtrList& modulePathList, unsigned int propertyFlags) const
{
  ModulePtrList tmpModuleList;
  ModulePtrList::const_iterator listIter;

  for (listIter = modulePathList.begin(); listIter != modulePathList.end(); listIter++) {
    Module* module = listIter->get();
    if (module->hasProperties(propertyFlags)) tmpModuleList.push_back(*listIter);
  }

  return tmpModuleList;
}


void ModuleChain::processInitialize(const ModulePtrList& modulePathList)
{
  LogSystem& logSystem = LogSystem::Instance();
  ModulePtrList::const_iterator listIter;

  for (listIter = modulePathList.begin(); listIter != modulePathList.end(); listIter++) {
    Module* module = listIter->get();

    if (module->hasUnsetForcedParams()) {
      ERROR("The module " << module->getType() << " has unset parameters which have to be set by the user !")
      continue;
    }

    //Set the module dependent log level
    logSystem.setLogLevel(module->getLogLevel());
    logSystem.setDebugLevel(module->getDebugLevel());

    //Do initialization
    module->initialize();
  }
}


void ModuleChain::processChain(PathPtr startPath, const ModulePtrList& modulePathList, long maxEvent)
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
      logSystem.setLogLevel(module->getLogLevel());
      logSystem.setDebugLevel(module->getDebugLevel());

      //Call the event method of the module
      module->event();

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
          } else WARNING("Module " << module->getType() << "requested to start a new run, but doesn't have the necessary property set. Request was ignored.");
          break;

        case Module::prt_EndRun    :
          //If the current module is allowed to trigger a new run, call the endRun() methods of all modules
          //and continue with the first module in the startPath.
          if (module->hasProperties(Module::c_TriggersNewRun)) {
            processEndRun(modulePathList);
            currPath = startPath;
            moduleIter = currPath->getModules().begin();
            normalEvent = false;
          } else WARNING("Module " << module->getType() << "requested to end a run, but doesn't have the necessary property set. Request was ignored.");
          break;

        case Module::prt_EndOfData :
          //If the current module is allowed to end the process and the end of data flow is reached, stop the event process.
          if (module->hasProperties(Module::c_TriggersEndOfData)) {
            endProcess = true;
          } else WARNING("Module " << module->getType() << "requested to stop the event processing (end of data), but doesn't have the necessary property set. Request was ignored.");
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

    if ((maxEvent > 0) && (currEvent >= maxEvent)) endProcess = true;
    currEvent++;
  }
}


void ModuleChain::processTerminate(const ModulePtrList& modulePathList)
{
  LogSystem& logSystem = LogSystem::Instance();
  ModulePtrList::const_reverse_iterator listIter;

  for (listIter = modulePathList.rbegin(); listIter != modulePathList.rend(); listIter++) {
    Module* module = listIter->get();

    //Set the module dependent log level
    logSystem.setLogLevel(module->getLogLevel());
    logSystem.setDebugLevel(module->getDebugLevel());

    //Do termination
    module->terminate();
  }

  //Delete persistent data in DataStore
  DataStore::Instance().clearMaps(c_Persistent);
}


void ModuleChain::processBeginRun(const ModulePtrList& modulePathList)
{
  LogSystem& logSystem = LogSystem::Instance();
  ModulePtrList::const_iterator listIter;

  for (listIter = modulePathList.begin(); listIter != modulePathList.end(); listIter++) {
    Module* module = listIter->get();

    //Set the module dependent log level
    logSystem.setLogLevel(module->getLogLevel());
    logSystem.setDebugLevel(module->getDebugLevel());

    //Do beginRun() call
    module->beginRun();
  }
}


void ModuleChain::processEndRun(const ModulePtrList& modulePathList)
{
  LogSystem& logSystem = LogSystem::Instance();
  ModulePtrList::const_iterator listIter;

  for (listIter = modulePathList.begin(); listIter != modulePathList.end(); listIter++) {
    Module* module = listIter->get();

    //Set the module dependent log level
    logSystem.setLogLevel(module->getLogLevel());
    logSystem.setDebugLevel(module->getDebugLevel());

    //Do endRun() call
    module->endRun();
  }

  //Delete run related data in DataStore
  DataStore::Instance().clearMaps(c_Run);
}


//============================================================================
//                              Private methods
//============================================================================

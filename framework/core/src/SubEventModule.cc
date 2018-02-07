/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Pulvermacher                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/core/SubEventModule.h>

#include <framework/core/ModuleManager.h>
#include <framework/core/Environment.h>
#include <framework/core/ProcessStatistics.h>
#include <framework/pcore/ProcHandler.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>

using namespace Belle2;

//REG_MODLUE needed for --execute-path functionality
//Note: should not appear in module list since we're not in the right directory
REG_MODULE(SubEvent)


SubEventModule::SubEventModule():
  Module(),
  EventProcessor(),
  m_objectName(""),
  m_loopOver(),
  m_path(),
  m_processID(-1)
{
  //since we might be created via 'new'...
  setDescription("Internal module to handle Path.for_each(). This shouldn't appear in 'basf2 -m' output. If it does, check REG_MODULE() handling.");

  addParam("loopOver", m_loopOverName, "Name of array to iterate over.", std::string(""));
  addParam("objectName", m_objectName, "Name of the object holding the current iteration's item.", std::string(""));
  addParam("path", m_path, "Path to execute for each iteration.", PathPtr(nullptr));
}

SubEventModule::~SubEventModule()
{
}

void SubEventModule::initSubEvent(const std::string& objectName, const std::string& loopOver, std::shared_ptr<Path> path)
{
  m_objectName = objectName;
  m_loopOverName = loopOver;
  m_path = path;
  setName("for_each(" + m_objectName + " : " + m_loopOverName + ")");

  m_moduleList = m_path->buildModulePathList();
  //set c_ParallelProcessingCertified flag if _all_ modules have it set
  auto flag = Module::c_ParallelProcessingCertified;
  if (ModuleManager::allModulesHaveFlag(m_moduleList, flag))
    setPropertyFlags(c_TerminateInAllProcesses | flag);
  else
    setPropertyFlags(c_TerminateInAllProcesses);
}

void restoreContents(const DataStore::StoreEntryMap& orig, DataStore::StoreEntryMap& dest)
{
  for (auto entry : orig) {
    auto& destEntry = dest[entry.first];
    auto& srcEntry = entry.second;
    if (srcEntry.ptr == nullptr)
      destEntry.ptr = nullptr;
  }
}


void SubEventModule::initialize()
{
  if (!m_path) {
    B2FATAL("SubEvent module not initialised properly.");
  }
  m_loopOver.isRequired(m_loopOverName);

  StoreObjPtr<ProcessStatistics> processStatistics("", DataStore::c_Persistent);
  processStatistics->suspendGlobal();

  //register loop object (c_DontWriteOut to disable writing the object)
  const DataStore::StoreEntry& arrayEntry = DataStore::Instance().getStoreEntryMap(DataStore::c_Event).at(m_loopOver.getName());
  DataStore::Instance().registerEntry(m_objectName, DataStore::c_Event, arrayEntry.objClass, false, DataStore::c_DontWriteOut);

  m_moduleList = m_path->buildModulePathList();
  processInitialize(m_moduleList, false);

  //don't screw up statistics for this module
  processStatistics->startModule();
  processStatistics->resumeGlobal();
}

void SubEventModule::terminate()
{
  StoreObjPtr<ProcessStatistics> processStatistics("", DataStore::c_Persistent);
  processStatistics->suspendGlobal();

  //get event map and make a deep copy of the StoreEntry objects
  //(we want to revert changes to the StoreEntry objects, but not to the arrays/objects)
  DataStore::StoreEntryMap& persistentMap = DataStore::Instance().getStoreEntryMap(DataStore::c_Persistent);
  DataStore::StoreEntryMap persistentMapCopy = persistentMap;

  if (!ProcHandler::parallelProcessingUsed() or m_processID == ProcHandler::EvtProcID()) {
    processTerminate(m_moduleList);
  } else {
    //we're in another process than we actually belong to, only call terminate where approriate
    ModulePtrList tmpModuleList;
    for (const ModulePtr& m : m_moduleList) {
      if (m->hasProperties(c_TerminateInAllProcesses))
        tmpModuleList.push_back(m);
    }
    processTerminate(tmpModuleList);
  }

  restoreContents(persistentMapCopy, persistentMap);

  //don't screw up statistics for this module
  processStatistics->startModule();
  processStatistics->resumeGlobal();
}

void SubEventModule::beginRun()
{
  m_processID = ProcHandler::EvtProcID();

  StoreObjPtr<ProcessStatistics> processStatistics("", DataStore::c_Persistent);
  processStatistics->suspendGlobal();
  processBeginRun();

  //don't screw up statistics for this module
  processStatistics->startModule();
  processStatistics->resumeGlobal();
}
void SubEventModule::endRun()
{
  StoreObjPtr<ProcessStatistics> processStatistics("", DataStore::c_Persistent);
  processStatistics->suspendGlobal();
  processEndRun();

  //don't screw up statistics for this module
  processStatistics->startModule();
  processStatistics->resumeGlobal();
}

void SubEventModule::event()
{
  const int numEntries = m_loopOver.getEntries();
  // Nothing to do? fine, don't do anything
  if (numEntries == 0) return;

  //disable statistics for subevent
  const bool noStats = Environment::Instance().getNoStats();
  StoreObjPtr<ProcessStatistics> processStatistics("", DataStore::c_Persistent);
  //Environment::Instance().setNoStats(true);
  processStatistics->suspendGlobal();

  //get event map and make a deep copy of the StoreEntry objects
  //(we want to revert changes to the StoreEntry objects, but not to the arrays/objects)
  DataStore::StoreEntryMap& eventMap = DataStore::Instance().getStoreEntryMap(DataStore::c_Event);
  DataStore::StoreEntryMap eventMapCopy = eventMap;

  DataStore::StoreEntry& objectEntry = DataStore::Instance().getStoreEntryMap(DataStore::c_Event).at(m_objectName);

  //don't call processBeginRun/EndRun() again (we do that in our implementations)
  m_previousEventMetaData = *(StoreObjPtr<EventMetaData>());

  //remember the state of the object before we loop over it
  TObject* prevObject = objectEntry.object;
  TObject* prevPtr = objectEntry.ptr;

  for (int i = 0; i < numEntries; i++) {
    //set loopObject
    objectEntry.object = m_loopOver[i];
    objectEntry.ptr = m_loopOver[i];

    //stuff usually done in processCore()
    PathIterator moduleIter(m_path);
    processEvent(moduleIter, false);

    //restore datastore
    restoreContents(eventMapCopy, eventMap);
  }

  // Let's reset the object to the way it was
  objectEntry.object = prevObject;
  objectEntry.ptr = prevPtr;

  Environment::Instance().setNoStats(noStats);

  //don't screw up statistics for this module
  processStatistics->startModule();
  processStatistics->resumeGlobal();
}

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

#include <framework/core/Environment.h>
#include <framework/core/ProcessStatistics.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>


using namespace Belle2;


//module not registered, framework internal


SubEventModule::SubEventModule(const std::string& objectName, const std::string& loopOver, boost::shared_ptr<Path> path):
  Module(),
  EventProcessor(),
  m_objectName(objectName),
  m_loopOver(loopOver),
  m_path(path)
{
  setModuleName("for_each(" + objectName + " : " + loopOver + ")");
  setDescription("Executes the given path for all entries in array 'loopOver'");

  m_moduleList = path->buildModulePathList();
  //set c_ParallelProcessingCertified flag if _all_ modules have it set
  bool allCertified = true;
  for (const auto & mod : m_moduleList) {
    if (!mod->hasProperties(c_ParallelProcessingCertified)) {
      allCertified = false;
      break;
    }
  }
  if (allCertified)
    setPropertyFlags(c_ParallelProcessingCertified);
}

SubEventModule::~SubEventModule()
{
}

void SubEventModule::initialize()
{
  m_loopOver.isRequired();

  ProcessStatistics::getInstance().suspendGlobal();

  //register loop object (!array, transient, errorIfExisting)
  const DataStore::StoreEntry* arrayEntry = DataStore::Instance().getStoreObjectMap(DataStore::c_Event).at(m_loopOver.getName());
  TClass* arrayClass = static_cast<TClonesArray*>(arrayEntry->object)->GetClass();
  DataStore::Instance().registerEntry(m_objectName, DataStore::c_Event, arrayClass, false, true, true);

  processInitialize(m_moduleList);

  //yes, we're still in initalize()
  DataStore::Instance().setInitializeActive(true);

  //don't screw up statistics for this module
  ProcessStatistics::getInstance().startModule();
  ProcessStatistics::getInstance().resumeGlobal();
}

void SubEventModule::terminate()
{
  ProcessStatistics::getInstance().suspendGlobal();
  processTerminate(m_moduleList);

  //don't screw up statistics for this module
  ProcessStatistics::getInstance().startModule();
  ProcessStatistics::getInstance().resumeGlobal();
}

void SubEventModule::beginRun()
{
  ProcessStatistics::getInstance().suspendGlobal();
  processBeginRun();

  //don't screw up statistics for this module
  ProcessStatistics::getInstance().startModule();
  ProcessStatistics::getInstance().resumeGlobal();
}
void SubEventModule::endRun()
{
  ProcessStatistics::getInstance().suspendGlobal();
  processEndRun();

  //don't screw up statistics for this module
  ProcessStatistics::getInstance().startModule();
  ProcessStatistics::getInstance().resumeGlobal();
}

void deepCopy(const DataStore::StoreObjMap& orig, DataStore::StoreObjMap& dest)
{
  for (auto entry : orig) {
    dest[entry.first] = new DataStore::StoreEntry(*entry.second);
  }
}
void restoreContents(const DataStore::StoreObjMap& orig, DataStore::StoreObjMap& dest)
{
  for (auto entry : orig) {
    auto& destEntry = *dest[entry.first];
    auto& srcEntry = *entry.second;
    if (srcEntry.ptr == nullptr)
      destEntry.ptr = nullptr;
  }
}

void SubEventModule::event()
{
  //disable statistics for subevent
  const bool noStats = Environment::Instance().getNoStats();
  //Environment::Instance().setNoStats(true);
  ProcessStatistics::getInstance().suspendGlobal();

  const int numEntries = m_loopOver.getEntries();

  //get event map and make a deep copy of the StoreEntry objects
  //(we want to revert changes to the StoreEntry objects, but not to the arrays/objects)
  DataStore::StoreObjMap& eventMap = DataStore::Instance().getStoreObjectMap(DataStore::c_Event);
  DataStore::StoreObjMap eventMapCopy;
  deepCopy(eventMap, eventMapCopy);

  //processCore() resets EventMetaData
  StoreObjPtr<EventMetaData> eventMetaDataPtr;
  const EventMetaData eventMetaDataBack = *eventMetaDataPtr;

  DataStore::StoreEntry* objectEntry = DataStore::Instance().getStoreObjectMap(DataStore::c_Event).at(m_objectName);

  for (int i = 0; i < numEntries; i++) {
    //set loopObject
    objectEntry->object = m_loopOver[i];
    objectEntry->ptr = m_loopOver[i];

    //stuff usually done in processCore()
    PathIterator moduleIter(m_path);
    EventMetaData previousEventMetaData(eventMetaDataBack);
    processEvent(moduleIter, &previousEventMetaData);

    //restore datastore
    restoreContents(eventMapCopy, eventMap);
  }

  objectEntry->object = nullptr;
  objectEntry->ptr = nullptr;

  //cleanup
  for (auto entry : eventMapCopy) {
    delete entry.second;
  }

  Environment::Instance().setNoStats(noStats);

  //don't screw up statistics for this module
  ProcessStatistics::getInstance().startModule();
  ProcessStatistics::getInstance().resumeGlobal();
}

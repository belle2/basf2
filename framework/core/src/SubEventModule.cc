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


SubEventModule::SubEventModule(): Module(), EventProcessor()
{
  // we need to deserialize a pickled execution path so this module needs to work
  // when using Path.add_module() directly ...
  setDescription(R"DOC(Internal module to handle Path.for_each() and Path.do_while().

  Warning:
    Don't add this module directly with `Path.add_module` or
    `basf2.register_module` but use `Path.for_each()` and `Path.do_while()`

  This module shouldn't appear in ``basf2 -m`` output.
  If it does, check REG_MODULE() handling.)DOC");

  addParam("mode", m_mode, "SubEvent mode: 0 = for_each, 1 = do_while", m_mode);
  addParam("loopOver", m_loopOverName, "Name of array to iterate over.", m_loopOverName);
  addParam("objectName", m_objectName, "Name of the object holding the current iteration's item.", m_objectName);
  addParam("path", m_path, "Path to execute for each iteration.", PathPtr(nullptr));
  addParam("maxIterations", m_maxIterations, "Maximum iterations in case of do_while", m_maxIterations);
  addParam("loopCondition", m_loopConditionString, "Loop condition in case of do_while", m_loopConditionString);
}

SubEventModule::~SubEventModule()
{
}

void SubEventModule::initSubEvent(const std::string& objectName, const std::string& loopOver, std::shared_ptr<Path> path)
{
  m_objectName = objectName;
  m_loopOverName = loopOver;
  m_path = path;
  setName("for_each(" + objectName + " : " + loopOver + ")");
  setProperties();
}

void SubEventModule::initSubLoop(std::shared_ptr<Path> path, const std::string& condition, unsigned int maxIterations)
{
  m_mode = c_DoWhile;
  m_path = path;
  m_maxIterations = maxIterations;
  m_loopConditionString = condition.empty() ? "<1" : condition;
  setDoWhileConditions();
  setName("do_while(" + m_loopConditionModule->getName() + *m_loopConditionString + ")");
  setProperties();
}

void SubEventModule::setDoWhileConditions()
{
  if (!m_loopConditionString) {
    B2FATAL("No Loop conditions specified");
  }
  if (!m_loopCondition) {
    m_loopCondition.reset(new ModuleCondition(*m_loopConditionString, m_path, EAfterConditionPath::c_End));
  }
  // We also need the last module to get its return value. If it's already set then fine.
  if (m_loopConditionModule) return;

  // Otherwise loop over everything
  PathIterator iter(m_path);
  while (!iter.isDone()) {
    m_loopConditionModule = iter.get();
    // Sooo, we have one problem: If the module has a condition with c_End as
    // after condition path the execution does not resume on our loop path and
    // the final module would not be executed and thus the return value is not
    // set correctly. Now we could check for exactly this case and just refuse
    // to have c_End conditions in our loop path but even if the condition is
    // c_Continue, as soon as the conditional path contains another c_End
    // condition processing will stop there. Which is debatable but currently
    // the implementation. This means the only safe thing to do is to not allow any
    // conditions in our loop for now
    if (m_loopConditionModule->hasCondition()) {
      B2FATAL("Modules in a Path.do_while() cannot have any conditions");
      //If we fix the path behavior that c_Continue will always come back
      //correctly we could be less restrictive here and only disallow c_End
      //conditions as those would really lead to undefined loop conditions
      for (const auto& condition : m_loopConditionModule->getAllConditions()) {
        if (condition.getAfterConditionPath() == ModuleCondition::EAfterConditionPath::c_End) {
          B2FATAL("do_while(): Modules in a loop cannot have conditions which end processing");
        }
      }
    }
    iter.next();
  }
  //apparently no modules at all?
  if (!m_loopConditionModule) {
    B2FATAL("do_while(): Cannot loop over empty path");
  }

  // If the last module has a condition attached the looping cannot be done
  // reliably as event processing might take that conditional path.
  // Obviously this does nothing right now until we fix the nested
  // c_Continue->c_End inconsitency and allow some conditions
  if (m_loopConditionModule->hasCondition()) {
    B2FATAL("do_while(): The last module in the loop path (" <<
            m_loopConditionModule->getName() << ") cannot have a condition");
  }
}

void SubEventModule::setProperties()
{
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

  if (m_mode == c_DoWhile) {
    setDoWhileConditions();
  } else if (m_mode == c_ForEach) {
    if (!(m_objectName && m_loopOverName)) {
      B2FATAL("loopOver and objectName parameters not setup");
    }
  } else {
    B2FATAL("Invalid SubEvent mode: " << m_mode);
  }

  StoreObjPtr<ProcessStatistics> processStatistics("", DataStore::c_Persistent);
  processStatistics->suspendGlobal();

  if (m_mode == c_ForEach) {
    m_loopOver.isRequired(*m_loopOverName);
    //register loop object (c_DontWriteOut to disable writing the object)
    const DataStore::StoreEntry& arrayEntry = DataStore::Instance().getStoreEntryMap(DataStore::c_Event).at(m_loopOver.getName());
    DataStore::Instance().registerEntry(*m_objectName, DataStore::c_Event, arrayEntry.objClass, false, DataStore::c_DontWriteOut);
  }

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

  if (m_mode == c_ForEach) {
    restoreContents(persistentMapCopy, persistentMap);
  }

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
  // Nothing to do? fine, don't do anything;
  if (m_mode == c_ForEach && m_loopOver.getEntries() == 0) return;

  //disable statistics for subevent
  const bool noStats = Environment::Instance().getNoStats();
  StoreObjPtr<ProcessStatistics> processStatistics("", DataStore::c_Persistent);
  //Environment::Instance().setNoStats(true);
  processStatistics->suspendGlobal();

  //don't call processBeginRun/EndRun() again (we do that in our implementations)
  m_previousEventMetaData = *(StoreObjPtr<EventMetaData>());
  //and don't reinitialize the random numbers in this path
  m_master = nullptr;

  if (m_mode == c_ForEach) {
    //get event map and make a deep copy of the StoreEntry objects
    //(we want to revert changes to the StoreEntry objects, but not to the arrays/objects)
    DataStore::StoreEntryMap& eventMap = DataStore::Instance().getStoreEntryMap(DataStore::c_Event);
    DataStore::StoreEntryMap eventMapCopy = eventMap;
    DataStore::StoreEntry& objectEntry = DataStore::Instance().getStoreEntryMap(DataStore::c_Event).at(*m_objectName);


    //remember the state of the object before we loop over it
    TObject* prevObject = objectEntry.object;
    TObject* prevPtr = objectEntry.ptr;

    const int numEntries = m_loopOver.getEntries();
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
  } else {
    int returnValue{false};
    unsigned int curIteration{0};
    do {
      if (++curIteration > m_maxIterations) {
        B2FATAL(getName() << ": Maximum number of " << m_maxIterations << " iterations reached");
      }
      //stuff usually done in processCore()
      PathIterator moduleIter(m_path);
      processEvent(moduleIter, false);
      B2ASSERT("Module " << m_loopConditionModule->getName() << " did not set a return value, cannot loop",
               m_loopConditionModule->hasReturnValue());
      returnValue = m_loopConditionModule->getReturnValue();
    } while (m_loopCondition->evaluate(returnValue));
  }


  Environment::Instance().setNoStats(noStats);

  //don't screw up statistics for this module
  processStatistics->startModule();
  processStatistics->resumeGlobal();
}

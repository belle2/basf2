/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/core/MergeDataStoreModule.h>
#include <framework/datastore/DataStore.h>

#include <framework/core/Environment.h>
#include <framework/core/InputController.h>


using namespace Belle2;

//REG_MODLUE needed for --execute-path functionality
//Note: should not appear in module list since we're not in the right directory
REG_MODULE(MergeDataStore);

MergeDataStoreModule::MergeDataStoreModule() : Module()
{
  setDescription("Internal module used by Path.add_independent_merge_path(). This shouldn't appear in 'basf2 -m' output. If it does, check REG_MODULE() handling.");

  addParam("toID", m_to, "active DataStore id after this module", std::string(""));
  addParam("createNew", m_createNew,
           "do you want to create a new (empty) DataStore 'toID'? This should be true only when toID refers to a _new_ DataStore ID", false);
  addParam("mergeBack", m_mergeBack, "copy the given objects/arrays over even if createNew is false.",
           std::vector<std::string> {"EventMetaData"});
}

MergeDataStoreModule::~MergeDataStoreModule() = default;

void MergeDataStoreModule::init(const std::string& to, bool createNew, const std::vector<std::string>& mergeBack)
{
  m_to = to;
  m_createNew = createNew;
  m_mergeBack = mergeBack;
}

void MergeDataStoreModule::initialize()
{
  m_from = DataStore::Instance().currentID();
  if (m_from == m_to)
    B2FATAL("identical from/to parameter value " << m_from);
  if (m_createNew and m_to == "")
    B2FATAL("createNew is set for default DataStore ID! This would likely cause corruption.");
  if (not m_createNew and m_from == "")
    B2FATAL("createNew is not set ?");

  if (m_createNew) {
    //create DataStore ID that doesn't exist yet (copying contents)
    DataStore::Instance().createEmptyDataStoreID(m_to);
  } else if (!m_mergeBack.empty()) {
    //if m_mergeBack is set, we need to register the objects/arrays there, too (if they are not registered yet)
    DataStore::Instance().copyEntriesTo(m_to, m_mergeBack, true);
  }

  //switch
  DataStore::Instance().switchID(m_to);
}

void MergeDataStoreModule::terminate()
{
  //terminate() is called in reverse order.
  DataStore::Instance().switchID(m_from);
}

void MergeDataStoreModule::event()
{
  if (m_createNew) {
    // don't do anything
  } else if (!m_mergeBack.empty()) {
    DataStore::Instance().mergeContentsTo(m_to, m_mergeBack);
  }

  DataStore::Instance().switchID(m_to);
}

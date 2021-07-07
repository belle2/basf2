/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/core/SwitchDataStoreModule.h>
#include <framework/datastore/DataStore.h>


using namespace Belle2;

//REG_MODLUE needed for --execute-path functionality
//Note: should not appear in module list since we're not in the right directory
REG_MODULE(SwitchDataStore)

SwitchDataStoreModule::SwitchDataStoreModule() : Module()
{
  setDescription("Internal module used by Path.add_independent_path(). This shouldn't appear in 'basf2 -m' output. If it does, check REG_MODULE() handling.");

  addParam("toID", m_to, "active DataStore id after this module", std::string(""));
  addParam("doCopy", m_doCopy,
           "should data be copied to DataStore 'toID'? This should be true only when toID refers to a _new_ DataStore ID", false);
  addParam("mergeBack", m_mergeBack, "if given, copy the given objects/arrays over even if doCopy is fals.", std::vector<std::string> {});
}

SwitchDataStoreModule::~SwitchDataStoreModule() = default;
void SwitchDataStoreModule::init(const std::string& to, bool doCopy, const std::vector<std::string>& mergeBack)
{
  m_to = to;
  m_doCopy = doCopy;
  m_mergeBack = mergeBack;
}

void SwitchDataStoreModule::initialize()
{
  m_from = DataStore::Instance().currentID();
  if (m_from == m_to)
    B2FATAL("identical from/to parameter value " << m_from);
  if (m_doCopy and m_to == "")
    B2FATAL("doCopy is set for default DataStore ID! This would likely cause corruption.");
  if (not m_doCopy and m_from == "")
    B2FATAL("doCopy is not set ?");

  if (m_doCopy) {
    //create DataStore ID that doesn't exist yet (copying contents)
    DataStore::Instance().createNewDataStoreID(m_to);
  } else if (!m_mergeBack.empty()) {
    //if m_mergeBack is set, we need to register the objects/arrays there, too!
    DataStore::Instance().copyEntriesTo(m_to, m_mergeBack);
    //then copy
    DataStore::Instance().copyContentsTo(m_to, m_mergeBack);
  }

  //switch
  DataStore::Instance().switchID(m_to);
}
void SwitchDataStoreModule::terminate()
{
  if (not m_doCopy) {
    //copy contents over
    DataStore::Instance().copyContentsTo(m_from);
  }
  //nothing merged back. this is not really consistent anyway.

  //terminate() is called in reverse order.
  DataStore::Instance().switchID(m_from);
}

void SwitchDataStoreModule::beginRun()
{
  event();
}
void SwitchDataStoreModule::endRun()
{
  event();
}
void SwitchDataStoreModule::event()
{
  if (m_doCopy) {
    //copy contents over
    DataStore::Instance().copyContentsTo(m_to);
  } else if (!m_mergeBack.empty()) {
    DataStore::Instance().copyContentsTo(m_to, m_mergeBack);
  }

  DataStore::Instance().switchID(m_to);
}

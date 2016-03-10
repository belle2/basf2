/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Pulvermacher                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/core/SwitchDataStoreModule.h>
#include <framework/datastore/DataStore.h>


using namespace Belle2;

//Note: should not appear in module list since we're not in the right directory
REG_MODULE(SwitchDataStore)

SwitchDataStoreModule::SwitchDataStoreModule() : Module()
{
  setDescription("Internal module used by Path.add_skim_path(). This shouldn't appear in 'basf2 -m' output. If it does, check REG_MODULE() handling.");

  addParam("toID", m_to, "active DataStore id after this module", std::string(""));
  addParam("doCopy", m_doCopy,
           "should data be copied to DataStore 'toID'? This should be true only when toID refers to a _new_ DataStore ID", false);
}

SwitchDataStoreModule::~SwitchDataStoreModule()
{
}
void SwitchDataStoreModule::init(std::string to, bool doCopy)
{
  m_to = to;
  m_doCopy = doCopy;
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
  }

  DataStore::Instance().switchID(m_to);
}

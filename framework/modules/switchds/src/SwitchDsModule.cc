/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Pulvermacher                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/modules/switchds/SwitchDsModule.h>
#include <framework/datastore/DataStore.h>


using namespace Belle2;

REG_MODULE(SwitchDs)

SwitchDsModule::SwitchDsModule() : Module()
{
  setDescription("Internal module used by Path.add_skim_path(). Don't use it directly.");

  addParam("toID", m_to, "active DataStore id after this module");
  addParam("doCopy", m_doCopy,
           "should data be copied to DataStore 'toID'? This should be true only when toID refers to a _new_ DataStore ID");
}

SwitchDsModule::~SwitchDsModule()
{
}

void SwitchDsModule::initialize()
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
void SwitchDsModule::terminate()
{
  if (not m_doCopy) {
    //copy contents over
    DataStore::Instance().copyContentsTo(m_from);
  }

  //terminate() is called in reverse order.
  DataStore::Instance().switchID(m_from);
}

void SwitchDsModule::beginRun()
{
  event();
}
void SwitchDsModule::endRun()
{
  event();
}
void SwitchDsModule::event()
{
  if (m_doCopy) {
    //copy contents over
    DataStore::Instance().copyContentsTo(m_to);
  }

  DataStore::Instance().switchID(m_to);
}

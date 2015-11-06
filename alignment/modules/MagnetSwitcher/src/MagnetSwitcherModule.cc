/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: tadeas                                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <alignment/modules/MagnetSwitcher/MagnetSwitcherModule.h>
#include <alignment/gfbfield/GFGeant4RecoField.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>
#include <../framework/datastore/include/StoreArray.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(MagnetSwitcher)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

MagnetSwitcherModule::MagnetSwitcherModule() : Module()
{
  setPropertyFlags(c_ParallelProcessingCertified);
  // Set module properties
  setDescription("Module to switch magnetic field ON/OFF for reconstruction in GBLfit. DataStore object in DST overrides user settings");

  // Parameter definitions
  addParam("expList", m_expList, "List of experiments to switch magnet off (must be same size as runList", std::vector<int>());
  addParam("runList", m_runList, "List of runs to switch magnet off (must be same size as expList)", std::vector<int>());

}

void MagnetSwitcherModule::initialize()
{
  if (m_expList.size() != m_runList.size())
    B2FATAL("The size of list of runs does not match the size of list of experiments.");

  StoreArray<TNamed> magnetOffEvent("MagnetOffEvents", DataStore::c_Event);
  magnetOffEvent.isOptional();
}

void MagnetSwitcherModule::beginRun()
{
  StoreObjPtr<EventMetaData> emd;
  StoreArray<TNamed> magnetOffEvents("MagnetOffEvents", DataStore::c_Event);

  bool& switcher = GFGeant4RecoField::getSwitcher();
  // Per default magnet is ON
  switcher = true;
  // DST info overrides all parameter settings if available
  if (magnetOffEvents.isValid()) {
    if (magnetOffEvents.getEntries()) {
      switcher = false;
    }
    return;
  }

  // If current run in list, switch field OFF
  for (unsigned int i = 0; i < m_expList.size(); i++) {
    if (emd->getExperiment() == m_expList[i] && emd->getRun() == m_runList[i]) {
      switcher = false;
      break;
    }
  }
}



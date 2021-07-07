/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <pxd/modules/pxdUnpacking/PXDGatedInfoFillerModule.h>

using namespace Belle2;
using namespace Belle2::PXD;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PXDGatedInfoFiller)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PXDGatedInfoFillerModule::PXDGatedInfoFillerModule() : Module()
{
  //Set module properties
  setDescription("Fill Gates Mode Information from ... for Reconstruction");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("GatedModeInfoName", m_GatedModeInfoName, "The name of the StoreObject of GatedModeInfo", std::string(""));
}

void PXDGatedInfoFillerModule::initialize()
{
//  m_someobjwheretoreaddatafrom.isRequired();
  m_storeGatedModeInfo.registerInDataStore(m_GatedModeInfoName, DataStore::EStoreFlags::c_ErrorIfAlreadyRegistered);
}

void PXDGatedInfoFillerModule::event()
{
  m_storeGatedModeInfo.create();
  m_storeGatedModeInfo->setFullGated(false);
  m_storeGatedModeInfo->setReadoutGated(true);
  for (int i = 0; i < 192; i++) {
    m_storeGatedModeInfo->setGateGatedL2(i, true);
  }
}

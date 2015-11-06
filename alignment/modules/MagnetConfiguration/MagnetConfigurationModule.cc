/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: tadeas                                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <alignment/modules/MagnetConfiguration/MagnetConfigurationModule.h>
#include <framework/datastore/StoreArray.h>
#include <geometry/bfieldmap/BFieldMap.h>
#include <TNamed.h>


using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(MagnetConfiguration)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

MagnetConfigurationModule::MagnetConfigurationModule() : Module()
{
  // Set module properties
  setPropertyFlags(c_ParallelProcessingCertified);
  setDescription("Stores configuration of magnet for each event (nothing if magnet ON. Special object if OFF)");
  // Parameter definitions

}

void MagnetConfigurationModule::initialize()
{
  StoreArray<TNamed> magnetOffEvents("MagnetOffEvents", DataStore::c_Event);
  magnetOffEvents.registerInDataStore();
  magnetOffEvents.create();
}

void MagnetConfigurationModule::event()
{
  StoreArray<TNamed> magnetOffEvents("MagnetOffEvents", DataStore::c_Event);

  bool magnetOff = BFieldMap::Instance().getBField(TVector3(0., 0., 0.)).Mag() < 100 *  1.e-6;
  if (magnetOff)
    magnetOffEvents.appendNew("MagnetOFFevent", "In this event magnetic field at origin is less than 100 uT.");
}



/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/modules/pxdReconstruction/PXDRawHitProducerModule.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/logging/Logger.h>
#include <TRandom.h>

using namespace std;
using namespace Belle2;
using namespace Belle2::PXD;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PXDRawHitProducer);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PXDRawHitProducerModule::PXDRawHitProducerModule() : Module()
{
  //Set module properties
  setDescription("This module converts the input collection of PXDDigits into "
                 "a collection of PXDRawHits, and assigns the raw hits randomly to frames.");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("rawHits", m_storeRawHitsName, "PXDRawHit collection name", string(""));
  addParam("digits", m_storeDigitsName, "PXDDigit collection name", string(""));
}


void PXDRawHitProducerModule::initialize()
{
  //Register collections
  StoreArray<PXDRawHit> storeRawHits(m_storeRawHitsName);
  storeRawHits.registerInDataStore();

  StoreArray<PXDDigit> storeDigits(m_storeDigitsName);
  storeDigits.isRequired();

  m_storeRawHitsName = storeRawHits.getName();
  m_storeDigitsName = storeDigits.getName();
}

void PXDRawHitProducerModule::event()
{
  StoreArray<PXDRawHit> storeRawHits(m_storeRawHitsName);
  StoreArray<PXDDigit> storeDigits(m_storeDigitsName);

  // if no input, nothing to do
  if (!storeDigits || !storeDigits.getEntries()) return;

  //We need no sorting here, the digits come sorted from the digitizer.

  const int nRows = 768;
  VxdID currentSensorID(0);
  unsigned short frameCounter = 0;
  unsigned short startRow = 0;
  for (const PXDDigit& storeDigit : storeDigits) {
    VxdID sensorID = storeDigit.getSensorID();
    if (sensorID != currentSensorID) {
      // We are in a new sensor, so reset sensor-specific settings
      currentSensorID = sensorID;
      startRow = gRandom->Integer(nRows);
      frameCounter = 2;
    }
    if (frameCounter == 2 && storeDigit.getVCellID() >= startRow) frameCounter = 1;
    storeRawHits.appendNew(
      sensorID, storeDigit.getVCellID(), storeDigit.getUCellID(), storeDigit.getCharge(),
      startRow, frameCounter, 0);
  }
  // That's not all, folks. We have to destroy all current PXDDigits.
  storeDigits.clear();
}

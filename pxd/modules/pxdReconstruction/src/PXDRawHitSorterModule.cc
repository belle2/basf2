/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/modules/pxdReconstruction/PXDRawHitSorterModule.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/logging/Logger.h>

#include <pxd/reconstruction/Pixel.h>

#include <set>

using namespace std;
using namespace Belle2;
using namespace Belle2::PXD;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PXDRawHitSorter);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PXDRawHitSorterModule::PXDRawHitSorterModule() : Module()
{
  //Set module properties
  setDescription("This module converts the input collection of PXDRawHits into "
                 "a collection of PXDDigits sorted, for each half-ladder, by frame, row,"
                 "and column.");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("mergeDuplicates", m_mergeDuplicates, "If true, add charges of multiple instances of the same fired pixel. Otherwise only keep the first..", true);
  addParam("mergeFrames", m_mergeFrames, "If true, produce a single frame containing digits of all input frames.", true);
  addParam("zeroSuppressionCut", m_0cut, "Minimum charge for a digit to carry", -1000.0);
  addParam("acceptFake", m_acceptFake, "If true, VxdID 0 in rawhits will be replaced with 1.1.1.", false);
  addParam("rawHits", m_storeRawHitsName, "PXDRawHit collection name", string(""));
  addParam("digits", m_storeDigitsName, "PXDDigit collection name", string(""));
  addParam("frames", m_storeFramesName, "PXDFrames collection name", string(""));
}


void PXDRawHitSorterModule::initialize()
{
  //Register collections
  StoreArray<PXDRawHit> storeRawHits(m_storeRawHitsName);
  storeRawHits.required();

  StoreArray<PXDDigit> storeDigits(m_storeDigitsName);
  storeDigits.registerAsPersistent();

  StoreArray<PXDFrame> storeFrames(m_storeFramesName);
  storeFrames.registerAsPersistent();

  m_storeRawHitsName = storeRawHits.getName();
  m_storeDigitsName = storeDigits.getName();
  m_storeFramesName = storeFrames.getName();
}

void PXDRawHitSorterModule::event()
{
  StoreArray<PXDRawHit> storeRawHits(m_storeRawHitsName);
  StoreArray<PXDDigit> storeDigits(m_storeDigitsName);
  StoreArray<PXDFrame> storeFrames(m_storeFramesName);

  // if no input, nothing to do
  if (!storeRawHits || !storeRawHits.getEntries()) return;

  //Mapping of Pixel information to sort according to VxdID, row, column
  std::map<VxdID, std::multiset<Pixel>> sensors;
  std::map<VxdID, std::set<unsigned short>> startRows;

  // Fill sensor information to get sorted Pixel indices
  const int nPixels = storeRawHits.getEntries();
  unsigned short currentFrameNumber(0);
  VxdID currentSensorID(0);
  unsigned short frameCounter(1); // to recode frame numbers to small integers
  for (int i = 0; i < nPixels; i++) {
    const PXDRawHit* const rawhit = storeRawHits[i];
    // Zero-suppression cut
    if (rawhit->getCharge() < m_0cut) continue;
    Pixel px(rawhit, i);
    VxdID sensorID = rawhit->getSensorID();
    // For fake data, suuply a reasonable VxdID when VxdID is 0
    if (m_acceptFake && sensorID.getID() == 0) sensorID = 8480;
    if (sensorID != currentSensorID) {
      currentSensorID = sensorID;
      frameCounter = 1;
      currentFrameNumber = rawhit->getFrameNr();
    } else if (rawhit->getFrameNr() != currentFrameNumber) {
      frameCounter++;
      currentFrameNumber = rawhit->getFrameNr();
    }
    if (m_mergeFrames) frameCounter = 0;
    sensorID.setSegmentNumber(frameCounter); // should be 0 anyway...
    // We need some protection against crap data
    if (sensorID.getLayerNumber() && sensorID.getLadderNumber() && sensorID.getSensorNumber()) {
      sensors[sensorID].insert(px);
      startRows[sensorID].insert(rawhit->getStartRow());
    }
  }

  unsigned int index(0);
  // And just loop over the sensors and create the digits.
  for (auto & sensor : sensors) {
    const PXD::Pixel* lastpx(0);
    VxdID sensorID = sensor.first;
    for (const PXD::Pixel & px : sensor.second) {
      //Normal case: pixel has different address
      if (!lastpx || px > *lastpx) {
        //Write the digit
        storeDigits.appendNew(sensorID, px.getU(), px.getV(), 0, 0, px.getCharge());
        ++index;
      } else {
        //We already have a pixel at this address, see if we merge or drop the new one
        if (m_mergeDuplicates) {
          //Merge the two pixels. As the PXDDigit does not have setters we have to create a new object.
          const PXDDigit& old = *storeDigits[index - 1];
          *storeDigits[index - 1] = PXDDigit(old.getSensorID(), old.getUCellID(),
                                             old.getVCellID(), old.getUCellPosition(), old.getVCellPosition(),
                                             old.getCharge() + px.getCharge());
        } //Otherwise delete the second pixel by forgetting about it.
      }
      lastpx = &px;
      for (unsigned short startRow : startRows[sensorID])
        storeFrames.appendNew(sensorID, startRow);
    }
  }
}

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
#include <framework/datastore/RelationArray.h>
#include <framework/logging/Logger.h>

#include <vxd/geometry/GeoCache.h>

#include <pxd/reconstruction/Pixel.h>
#include <pxd/reconstruction/PXDPixelMasker.h>

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

  addParam("mergeDuplicates", m_mergeDuplicates,
           "If true, add charges of multiple instances of the same fired pixel. Otherwise only keep the first..", true);
  addParam("mergeFrames", m_mergeFrames, "If true, produce a single frame containing digits of all input frames.", true);
  addParam("zeroSuppressionCut", m_0cut, "Minimum charge for a digit to carry", 0);
  addParam("trimOutOfRange", m_trimOutOfRange, "Discard rawhits whith out-of-range coordinates", true);
  addParam("rawHits", m_storeRawHitsName, "PXDRawHit collection name", string(""));
  addParam("digits", m_storeDigitsName, "PXDDigit collection name", string(""));
}


void PXDRawHitSorterModule::initialize()
{
  //Register collections
  m_storeRawHits.isRequired(m_storeRawHitsName);
  //m_storeDigits.registerInDataStore(m_storeDigitsName, DataStore::c_ErrorIfAlreadyRegistered);
  m_storeDigits.registerInDataStore(m_storeDigitsName);
}

void PXDRawHitSorterModule::event()
{
  // if no input, nothing to do
  if (!m_storeRawHits || !m_storeRawHits.getEntries()) return;

  const VXD::GeoCache& geo = VXD::GeoCache::getInstance();

  //Mapping of Pixel information to sort according to VxdID, row, column
  std::map<VxdID, std::multiset<Pixel>> sensors;

  // Fill sensor information to get sorted Pixel indices
  const int nPixels = m_storeRawHits.getEntries();
  unsigned short currentFrameNumber(0);
  VxdID currentSensorID(0);
  // TODO: It seems there is no need for the frameCounter any more ... should remove it as well.
  unsigned short frameCounter(1); // to recode frame numbers to small integers
  for (int i = 0; i < nPixels; i++) {
    const PXDRawHit* const rawhit = m_storeRawHits[i];
    // If malformed object, drop it.
    VxdID sensorID = rawhit->getSensorID();
    if (!geo.validSensorID(sensorID)) {
      B2WARNING("Malformed PXDRawHit, VxdID $" << hex << sensorID.getID() << ", dropping. (" << sensorID << ")");
      continue;
    }
    if (m_trimOutOfRange && !goodHit(rawhit))
      continue;
    // Zero-suppression cut
    if (rawhit->getCharge() < m_0cut) continue;
    // FIXME: The index of the temporary Pixel object seems not needed here.
    // Set this index always to zero
    Pixel px(rawhit, 0);
    if (sensorID != currentSensorID) {
      currentSensorID = sensorID;
      frameCounter = 1;
      currentFrameNumber = rawhit->getFrameNr();
    } else if (rawhit->getFrameNr() != currentFrameNumber) {
      frameCounter++;
      currentFrameNumber = rawhit->getFrameNr();
    }
    if (m_mergeFrames) frameCounter = 0;

    // We need some protection against crap data
    if (sensorID.getLayerNumber() && sensorID.getLadderNumber() && sensorID.getSensorNumber()) {
      if (PXDPixelMasker::getInstance().pixelOK(sensorID, px.getU(), px.getV())) {
        sensors[sensorID].insert(px);
      }
    }
  }

  unsigned int index(0);
  // And just loop over the sensors and create the digits.
  for (auto& sensor : sensors) {
    const PXD::Pixel* lastpx(0);
    VxdID sensorID = sensor.first;
    for (const PXD::Pixel& px : sensor.second) {
      //Normal case: pixel has different address
      if (!lastpx || px > *lastpx) {
        //Write the digit
        m_storeDigits.appendNew(sensorID, px.getU(), px.getV(), px.getCharge());
        ++index;
      } else {
        //We already have a pixel at this address, see if we merge or drop the new one
        if (m_mergeDuplicates) {
          //Merge the two pixels. As the PXDDigit does not have setters we have to create a new object.
          const PXDDigit& old = *m_storeDigits[index - 1];
          *m_storeDigits[index - 1] = PXDDigit(old.getSensorID(), old.getUCellID(),
                                               old.getVCellID(), old.getCharge() + px.getCharge());
        } //Otherwise delete the second pixel by forgetting about it.
      }
      lastpx = &px;
    }
  }
}

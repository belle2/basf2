/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <pxd/modules/pxdReconstruction/PXDRawHitSorterModule.h>

#include <framework/logging/Logger.h>

#include <vxd/geometry/GeoCache.h>

#include <pxd/reconstruction/Pixel.h>
#include <pxd/reconstruction/PXDPixelMasker.h>

#include <set>
#include <algorithm>

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
           "If true, take maximum charge of multiple instances of the same fired pixel. Otherwise only keep the first.", true);
  addParam("zeroSuppressionCut", m_0cut, "Minimum charge for a digit to carry", 0);
  addParam("trimOutOfRange", m_trimOutOfRange, "Discard rawhits whith out-of-range coordinates", true);
  addParam("rawHits", m_storeRawHitsName, "PXDRawHit collection name", string(""));
  addParam("digits", m_storeDigitsName, "PXDDigit collection name", string(""));
}


void PXDRawHitSorterModule::initialize()
{
  //Register collections
  m_storeRawHits.isRequired(m_storeRawHitsName);
  m_storeDaqStatus.isRequired();
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
  // Get Map of (un)usable modules
  auto usability = m_storeDaqStatus->getUsable();

  // Fill sensor information to get sorted Pixel indices
  const int nPixels = m_storeRawHits.getEntries();
  for (int i = 0; i < nPixels; i++) {
    const PXDRawHit* const rawhit = m_storeRawHits[i];
    // If malformed object, drop it.
    VxdID sensorID = rawhit->getSensorID();
    if (!geo.validSensorID(sensorID)) {
      B2WARNING("Malformed PXDRawHit, VxdID $" << hex << sensorID.getID() << ", dropping. (" << sensorID << ")");
      continue;
    }
    if (!usability[sensorID]) continue;// masked as bad sensor data
    if (m_trimOutOfRange && !goodHit(rawhit))
      continue;
    // Zero-suppression cut
    if (rawhit->getCharge() < m_0cut) continue;
    // FIXME: The index of the temporary Pixel object seems not needed here.
    // Set this index always to zero
    Pixel px(rawhit, 0);
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
          //The charge of the new PXDDigit is the maximum of the duplicates.
          const PXDDigit& old = *m_storeDigits[index - 1];
          *m_storeDigits[index - 1] = PXDDigit(old.getSensorID(), old.getUCellID(),
                                               old.getVCellID(), std::max(old.getCharge(), static_cast<unsigned short>(px.getCharge())));
        } //Otherwise delete the second pixel by forgetting about it.
      }
      lastpx = &px;
    }
  }
}

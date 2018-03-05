/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/modules/pxdReconstruction/PXDRawHitMaskingModule.h>

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
REG_MODULE(PXDRawHitMasking);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PXDRawHitMaskingModule::PXDRawHitMaskingModule() : Module()
{
  //Set module properties
  setDescription("This module masks the input collection of PXDRawHits into "
                 "a PXDRawHit output collection");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("zeroSuppressionCut", m_0cut, "Minimum charge for a digit to carry", 0);
  addParam("trimOutOfRange", m_trimOutOfRange, "Discard rawhits whith out-of-range coordinates", true);
  addParam("rawHits", m_storeRawHitsName, "PXDRawHit collection name", string(""));
  addParam("rawHitsOut", m_storeRawHitsNameOut, "PXDRawHit Out collection name", string("PXDRawHitsOut"));
}


void PXDRawHitMaskingModule::initialize()
{
  //Register collections
  m_pxdRawHit.isRequired(m_storeRawHitsName);
  m_pxdRawHitOut.registerInDataStore(m_storeRawHitsNameOut);
}

void PXDRawHitMaskingModule::event()
{

  // if no input, nothing to do
  if (!m_pxdRawHit || !m_pxdRawHit.getEntries()) return;

  const VXD::GeoCache& geo = VXD::GeoCache::getInstance();

  for (auto& it : m_pxdRawHit) {
    VxdID sensorID = it.getSensorID();
    if (!geo.validSensorID(sensorID)) {
      B2WARNING("Malformed PXDRawHit, VxdID $" << hex << sensorID.getID() << ", dropping. (" << sensorID << ")");
      continue;
    }
    if (m_trimOutOfRange && !goodHit(it))
      continue;
    // Zero-suppression cut
    if (it.getCharge() < m_0cut) continue;

    // We need some protection against crap data
    if (sensorID.getLayerNumber() && sensorID.getLadderNumber() && sensorID.getSensorNumber()) {
      if (PXDPixelMasker::getInstance().pixelOK(sensorID, it.getColumn(), it.getRow())) {
        m_pxdRawHitOut.appendNew(it);
      }
    }
  }
}

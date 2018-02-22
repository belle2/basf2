/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Schwenker                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/modules/pxdHotPixelMaskCollector/PXDRawHotPixelMaskCollectorModule.h>
#include <vxd/geometry/GeoCache.h>

#include <TH1I.h>

using namespace std;
using namespace Belle2;


//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PXDRawHotPixelMaskCollector)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PXDRawHotPixelMaskCollectorModule::PXDRawHotPixelMaskCollectorModule() : CalibrationCollectorModule()
{
  // Set module properties
  setDescription("Calibration Collector Module for PXD hot pixel masking from rawhits");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("zeroSuppressionCut", m_0cut, "Minimum charge (in ADU) for detecting a hit", 0);
  addParam("rawHits", m_storeRawHitsName, "PXDRawHit collection name", string(""));
}

void PXDRawHotPixelMaskCollectorModule::prepare() // Do your initialise() stuff here
{
  m_pxdRawHit.isRequired();

  // Data object creation --------------------------------------------------
  auto hhitmap = new TH1I("hhitmap",
                          "Pixel hits from PXDDigits histogram, distribution parameters found by PXDRawHotPixelMaskCollectorModule", 250 * 768, 0, 250 * 768);

  auto hnevents = new TH1I("hnevents",
                           "Number of events used for masking, distribution parameters found by PXDRawHotPixelMaskCollectorModule", 1, 0, 1);

  // Data object registration ----------------------------------------------
  registerObject<TH1I>("hitmap", hhitmap); // Does the registerInDatastore for you
  registerObject<TH1I>("nevents", hnevents);

}

void PXDRawHotPixelMaskCollectorModule::collect() // Do your event() stuff here
{
  // if no input, nothing to do
  if (!m_pxdRawHit || !m_pxdRawHit.getEntries()) return;

  // Data object access and filling ----------------------------------------
  TH1I* collector_nevents = getObjectPtr<TH1I>("nevents");
  collector_nevents->Fill(0);

  TH1I* collector_hitmap = getObjectPtr<TH1I>("hitmap");

  const VXD::GeoCache& geo = VXD::GeoCache::getInstance();

  for (auto& rawhit :  m_pxdRawHit) {

    VxdID sensorID = rawhit.getSensorID();
    if (!geo.validSensorID(sensorID)) {
      B2WARNING("Malformed PXDRawHit, VxdID $" << hex << sensorID.getID() << ", dropping. (" << sensorID << ")");
      continue;
    }

    // We need some protection against crap data
    if (!goodHit(rawhit)) continue;

    // Zero-suppression cut
    if (rawhit.getCharge() < m_0cut) continue;

    // Increment counter for hit pixel
    if (sensorID.getLayerNumber() && sensorID.getLadderNumber() && sensorID.getSensorNumber()) {
      collector_hitmap->Fill(rawhit.getColumn() * 768 + rawhit.getRow());
    }
  }
}

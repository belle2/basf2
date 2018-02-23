/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Schwenker, Peter Kodys                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/modules/pxdHotPixelMaskCollector/PXDRawHotPixelMaskCollectorModule.h>
#include <vxd/geometry/GeoCache.h>

#include <boost/format.hpp>

#include <TH1I.h>

using namespace std;
using boost::format;
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

  auto gTools = VXD::GeoCache::getInstance().getGeoTools();

  if (gTools->getNumberOfPXDLayers() == 0) {
    B2WARNING("Missing geometry for PXD, PXD-masking is skiped.");
  }

  //-------------------------------------------------------------------------------------------------
  // PXDHits: Histogram the number of PXDRawHits per event (assuming Belle 2 PXD with < 2% occupancy)
  //-------------------------------------------------------------------------------------------------
  auto hPXDHits = new TH1I("hPXDHits",
                           "Number of hits in PXD per events used for masking, distribution parameters found by PXDRawHotPixelMaskCollectorModule", 200000, 0,
                           200000);
  hPXDHits->GetXaxis()->SetTitle("Number of hits");
  hPXDHits->GetYaxis()->SetTitle("Events");
  registerObject<TH1I>("PXDHits", hPXDHits);

  //--------------------------------------------------------
  // PXDHitCounts: Count the number of PXDRawHits per sensor
  //--------------------------------------------------------
  int nPXDSensors = gTools->getNumberOfPXDSensors();
  auto hPXDHitCounts = new TH1I("hPXDHitCounts",
                                "Number of hits in PXD sensors for masking, distribution parameters found by PXDRawHotPixelMaskCollectorModule", nPXDSensors, 0,
                                nPXDSensors);

  hPXDHitCounts->GetXaxis()->SetTitle("SensorID");
  hPXDHitCounts->GetYaxis()->SetTitle("Number of hits");
  for (int i = 0; i < nPXDSensors; i++) {
    VxdID id = gTools->getSensorIDFromPXDIndex(i);
    string sensorDescr = id;
    hPXDHitCounts->GetXaxis()->SetBinLabel(i + 1, str(format("%1%") % sensorDescr).c_str());
  }
  registerObject<TH1I>("PXDHitCounts", hPXDHitCounts);

  // Fill PXDHitMap with empty histos for all sensors
  for (int i = 0; i < nPXDSensors; i++) {
    VxdID id = gTools->getSensorIDFromPXDIndex(i);
    string sensorDescr = id;

    //----------------------------------------------------------------
    // Hitmaps: Number of hits per sensor and pixel
    //----------------------------------------------------------------
    string name = str(format("PXD_%1%_PixelHitmap") % id.getID());
    string title = str(format("PXD Sensor %1% Pixel Hitmap from PXDRawHotPixelMaskCollector") % sensorDescr);

    // Data object creation --------------------------------------------------
    auto hsensorhitmap = new TH1I(name.c_str(), title.c_str(), 250 * 768, 0, 250 * 768);
    registerObject<TH1I>(name.c_str(), hsensorhitmap);
  }
}

void PXDRawHotPixelMaskCollectorModule::collect() // Do your event() stuff here
{
  // Histogram pxd hits per event
  TH1I* collector_hits = getObjectPtr<TH1I>("PXDHits");

  // Even if there is no input StoreArray, we still want to fill zero hits
  if (!m_pxdRawHit)
    collector_hits->Fill(0);
  else
    collector_hits->Fill(m_pxdRawHit.getEntries());

  auto& geo = VXD::GeoCache::getInstance();
  auto gTools = geo.getGeoTools();

  // Count hits per sensor
  TH1I* collector_pxdhitcounts = getObjectPtr<TH1I>("PXDHitCounts");

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
      string name = str(format("PXD_%1%_PixelHitmap") % sensorID.getID());
      TH1I* collector_sensorhitmap = getObjectPtr<TH1I>(name.c_str());
      collector_sensorhitmap->Fill(rawhit.getColumn() * 768 + rawhit.getRow());
      collector_pxdhitcounts->Fill(gTools->getPXDSensorIndex(sensorID));
    }
  }
}

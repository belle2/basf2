/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Schwenker                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/modules/pxdHotPixelMaskCollector/PXDHotPixelMaskCollectorModule.h>
#include <vxd/geometry/GeoCache.h>

#include <boost/format.hpp>

#include <TH1I.h>

using namespace std;
using boost::format;
using namespace Belle2;


//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PXDHotPixelMaskCollector)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PXDHotPixelMaskCollectorModule::PXDHotPixelMaskCollectorModule() : CalibrationCollectorModule()
{
  // Set module properties
  setDescription("Calibration Collector Module for PXD hot pixel masking from digits");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("zeroSuppressionCut", m_0cut, "Minimum charge (in ADU) for detecting a hit", 0);
  addParam("digitsName", m_storeDigitsName, "PXDDigit collection name", string(""));
}

void PXDHotPixelMaskCollectorModule::prepare() // Do your initialise() stuff here
{
  m_pxdDigit.isRequired();

  auto gTools = VXD::GeoCache::getInstance().getGeoTools();

  if (gTools->getNumberOfPXDLayers() == 0) {
    B2WARNING("Missing geometry for PXD, PXD-masking is skiped.");
  }

  //-------------------------------------------------------------------------------------------------
  // PXDHits: Histogram the number of PXDDigits per event (assuming Belle 2 PXD with < 2% occupancy)
  //-------------------------------------------------------------------------------------------------
  auto hPXDHits = new TH1I("hPXDHits",
                           "Number of hits in PXD per events used for masking, distribution parameters found by PXDHotPixelMaskCollectorModule", 200000, 0,
                           200000);
  hPXDHits->GetXaxis()->SetTitle("Number of hits");
  hPXDHits->GetYaxis()->SetTitle("Events");
  registerObject<TH1I>("PXDHits", hPXDHits);

  //--------------------------------------------------------
  // PXDHitCounts: Count the number of PXDDigits per sensor
  //--------------------------------------------------------
  int nPXDSensors = gTools->getNumberOfPXDSensors();
  auto hPXDHitCounts = new TH1I("hPXDHitCounts",
                                "Number of hits in PXD sensors for masking, distribution parameters found by PXDHotPixelMaskCollectorModule", nPXDSensors, 0,
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
    string title = str(format("PXD Sensor %1% Pixel Hitmap from PXDHotPixelMaskCollector") % sensorDescr);

    // Data object creation --------------------------------------------------
    auto hsensorhitmap = new TH1I(name.c_str(), title.c_str(), 250 * 768, 0, 250 * 768);

    // Data object registration ----------------------------------------------
    registerObject<TH1I>(name.c_str(), hsensorhitmap); // Does the registerInDatastore for you
  }

}

void PXDHotPixelMaskCollectorModule::collect() // Do your event() stuff here
{

  // Histogram pxd hits per event
  TH1I* collector_hits = getObjectPtr<TH1I>("PXDHits");

  // Even if there is no input StoreArray, we still want to fill zero hits
  if (!m_pxdDigit)
    collector_hits->Fill(0);
  else
    collector_hits->Fill(m_pxdDigit.getEntries());

  auto& geo = VXD::GeoCache::getInstance();
  auto gTools = geo.getGeoTools();

  // Count hits per sensor
  TH1I* collector_pxdhitcounts = getObjectPtr<TH1I>("PXDHitCounts");

  for (auto& digit :  m_pxdDigit) {
    // Zero-suppression cut
    if (digit.getCharge() < m_0cut) continue;

    // Increment counter for hit pixel
    string name = str(format("PXD_%1%_PixelHitmap") % digit.getSensorID().getID());
    TH1I* collector_sensorhitmap = getObjectPtr<TH1I>(name.c_str());
    collector_sensorhitmap->Fill(digit.getUCellID() * 768 + digit.getVCellID());
    collector_pxdhitcounts->Fill(gTools->getPXDSensorIndex(digit.getSensorID()));
  }
}

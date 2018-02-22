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

  //----------------------------------------------------------------------------------------
  // Nevents: Count number of events which are accumulated (later for normalization of hits)
  //----------------------------------------------------------------------------------------
  auto hnevents = new TH1I("hnevents",
                           "Number of events used for masking, distribution parameters found by PXDRawHotPixelMaskCollectorModule", 1, 0, 1);

  registerObject<TH1I>("nevents", hnevents);

  int nPXDSensors = gTools->getNumberOfPXDSensors();

  for (int i = 0; i < nPXDSensors; i++) {
    VxdID id = gTools->getSensorIDFromPXDIndex(i);
    int iLayer = id.getLayerNumber();
    int iLadder = id.getLadderNumber();
    int iSensor = id.getSensorNumber();
    VxdID sensorID(iLayer, iLadder, iSensor);
    string sensorDescr = str(format("%1%_%2%_%3%") % iLayer % iLadder % iSensor);

    //----------------------------------------------------------------
    // Hitmaps: Number of hits per sensor and pixel
    //----------------------------------------------------------------
    string name = str(format("PXD_%1%_PixelHitmap") % i);
    string title = str(format("PXD Sensor %1% Pixel Hitmap from PXDRawHotPixelMaskCollector") % sensorDescr);

    // Data object creation --------------------------------------------------
    auto hpxdhitmap = new TH1I(name.c_str(), title.c_str(), 250 * 768, 0, 250 * 768);

    // Data object registration ----------------------------------------------
    // FIXME: is there a smarter way to register vector of histograms???
    registerObject<TH1I>(name.c_str(), hpxdhitmap); // Does the registerInDatastore for you
  }
}

void PXDRawHotPixelMaskCollectorModule::collect() // Do your event() stuff here
{
  // if no input, nothing to do
  if (!m_pxdRawHit || !m_pxdRawHit.getEntries()) return;

  auto& geo = VXD::GeoCache::getInstance();
  auto gTools = VXD::GeoCache::getInstance().getGeoTools();

  // Data object access and filling ----------------------------------------
  TH1I* collector_nevents = getObjectPtr<TH1I>("nevents");
  collector_nevents->Fill(0);

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
      int index = gTools->getPXDSensorIndex(sensorID);
      string name = str(format("PXD_%1%_PixelHitmap") % index);

      TH1I* collector_hitmap = getObjectPtr<TH1I>(name.c_str());
      collector_hitmap->Fill(rawhit.getColumn() * 768 + rawhit.getRow());
    }
  }
}

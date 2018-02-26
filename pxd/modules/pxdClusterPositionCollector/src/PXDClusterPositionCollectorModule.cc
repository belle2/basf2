/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Schwenker                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/modules/pxdClusterPositionCollector/PXDClusterPositionCollectorModule.h>
#include <vxd/geometry/GeoCache.h>

#include <boost/format.hpp>

#include <TH1I.h>

using namespace std;
using boost::format;
using namespace Belle2;


//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PXDClusterPositionCollector)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PXDClusterPositionCollectorModule::PXDClusterPositionCollectorModule() : CalibrationCollectorModule()
{
  // Set module properties
  setDescription("Calibration collector module for PXD cluster position estimation");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("clustersName", m_storeClustersName, "Name of the collection to use for PXDClusters", string(""));
  addParam("trueHitsName", m_storeTrueHitsName, "Name of the collection to use for PXDTrueHits", string(""));
}

void PXDClusterPositionCollectorModule::prepare() // Do your initialise() stuff here
{
  m_pxdCluster.isRequired();
  m_pxdTrueHit.isRequired();

  /*
  auto gTools = VXD::GeoCache::getInstance().getGeoTools();

  if (gTools->getNumberOfPXDLayers() == 0) {
    B2WARNING("Missing geometry for PXD, PXD cluster position estimation is skiped.");
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
  */
}

void PXDClusterPositionCollectorModule::collect() // Do your event() stuff here
{
  // If no input, nothing to do
  if (!m_pxdCluster || !m_pxdTrueHit) return;

  /*
  auto& geo = VXD::GeoCache::getInstance();
  auto gTools = geo.getGeoTools();


  for (auto& cluster :  m_pxdCluster) {

    // Increment counter for hit pixel
    //string name = str(format("PXD_%1%_PixelHitmap") % cluster.getSensorID().getID());
    //TH1I* collector_sensorhitmap = getObjectPtr<TH1I>(name.c_str());
    //collector_sensorhitmap->Fill(cluster.getU() * 768 + cluster.getV());
  }
  */
}

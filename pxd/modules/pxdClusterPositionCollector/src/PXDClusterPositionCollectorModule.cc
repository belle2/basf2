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
#include <pxd/dataobjects/PXDDigit.h>
#include <pxd/dataobjects/PXDTrueHit.h>
#include <vxd/geometry/GeoCache.h>

#include <boost/format.hpp>

#include <TTree.h>
#include <TMath.h>
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
}

void PXDClusterPositionCollectorModule::prepare() // Do your initialise() stuff here
{
  m_pxdCluster.isRequired();

  // Data object creation --------------------------------------------------
  auto tree = new TTree("PXDCluserPositionCalibration", "PXD Cluser Position Calibration Source Data");
  m_rootPxdClusterArray = new TClonesArray("Belle2::PXDCluster");
  m_rootPxdDigitArray = new TClonesArray("Belle2::PXDDigit");
  m_rootPxdTrueHitArray = new TClonesArray("Belle2::PXDTrueHit");
  tree->Branch<TClonesArray>("PXDClusterArray", &m_rootPxdClusterArray);
  tree->Branch<TClonesArray>("PXDDigitArray", &m_rootPxdDigitArray);
  tree->Branch<TClonesArray>("PXDTrueHitArray", &m_rootPxdTrueHitArray);
  registerObject<TTree>("pxdCal", tree);
}

void PXDClusterPositionCollectorModule::collect() // Do your event() stuff here
{
  // If no input, nothing to do
  if (!m_pxdCluster) return;

  for (auto& cluster :  m_pxdCluster) {

    // Ignore clustes with more than one truehit
    if (cluster.getRelationsTo<PXDTrueHit>().size() > 1) {
      continue;
    }

    for (auto& truehit : cluster.getRelationsTo<PXDTrueHit>()) {

      // Clear root arrays
      TClonesArray& pxdClusterArray = *m_rootPxdClusterArray;
      pxdClusterArray.Clear();
      TClonesArray& pxdTrueHitArray = *m_rootPxdTrueHitArray;
      pxdTrueHitArray.Clear();
      TClonesArray& pxdDigitArray = *m_rootPxdDigitArray;
      pxdDigitArray.Clear();

      //Get Geometry information
      auto sensorID = cluster.getSensorID();

      // Fill branche of output tree
      new(pxdClusterArray[0]) PXDCluster(cluster);
      new(pxdTrueHitArray[0]) PXDTrueHit(truehit);


      // Fill all digits related to cluster in TClonesArray
      for (int i = 0; i < cluster.getSize(); i++) {
        const PXDDigit* const storeDigit = cluster.getRelationsTo<PXDDigit>("PXDDigits")[i];
        new(pxdDigitArray[i]) PXDDigit(*storeDigit);
      }

      // Fill the tree
      getObjectPtr<TTree>("pxdCal")->Fill();

    }
  }

}

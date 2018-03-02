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

#include <TTree.h>
#include <TMath.h>

using namespace std;
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
  m_rootPxdClusterArray = new TClonesArray("Belle2::PXDCluster", 1);
  m_rootPxdDigitArray = new TClonesArray("Belle2::PXDDigit", 1000);
  m_rootPxdTrueHitArray = new TClonesArray("Belle2::PXDTrueHit", 1);
  tree->Branch<TClonesArray>("PXDClusterArray", &m_rootPxdClusterArray);
  tree->Branch<TClonesArray>("PXDDigitArray", &m_rootPxdDigitArray);
  tree->Branch<TClonesArray>("PXDTrueHitArray", &m_rootPxdTrueHitArray);
  registerObject<TTree>("pxdCal", tree);
}

void PXDClusterPositionCollectorModule::collect() // Do your event() stuff here
{
  // If no input, nothing to do
  if (!m_pxdCluster) return;

  TClonesArray& pxdClusterArray = *m_rootPxdClusterArray;
  TClonesArray& pxdTrueHitArray = *m_rootPxdTrueHitArray;
  TClonesArray& pxdDigitArray = *m_rootPxdDigitArray;

  for (auto& cluster :  m_pxdCluster) {

    // Ignore clustes with more than one truehit
    if (cluster.getRelationsTo<PXDTrueHit>().size() > 1) {
      continue;
    }

    // Ignore freak clustes with more than 1000 digits
    if (cluster.getSize() >= 1000) {
      continue;
    }

    for (auto& truehit : cluster.getRelationsTo<PXDTrueHit>()) {

      // Clear root arrays
      pxdClusterArray.Clear();
      pxdTrueHitArray.Clear();
      pxdDigitArray.Clear();

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

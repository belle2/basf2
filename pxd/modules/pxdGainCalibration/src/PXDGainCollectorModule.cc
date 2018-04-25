/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Schwenker                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/modules/pxdGainCalibration/PXDGainCollectorModule.h>
#include <vxd/geometry/GeoCache.h>
#include <pxd/geometry/SensorInfo.h>
#include <pxd/reconstruction/PXDClusterPositionEstimator.h>

#include <TTree.h>

#include <boost/format.hpp>
#include <cmath>

using namespace std;
using boost::format;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PXDGainCollector)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PXDGainCollectorModule::PXDGainCollectorModule() : CalibrationCollectorModule()
{
  // Set module properties
  setDescription("Calibration collector module for PXD gain calibration");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("clustersName", m_storeClustersName, "Name of the collection to use for PXDClusters", string(""));
  addParam("minClusterCharge", m_minClusterCharge, "Minimum cluster charge cut", int(20));
  addParam("minClusterSize", m_minClusterSize, "Minimum cluster size cut ", int(2));
}

void PXDGainCollectorModule::prepare() // Do your initialise() stuff here
{
  m_pxdCluster.isRequired();

  string treename = string("tree");
  auto tree = new TTree(treename.c_str(), treename.c_str());
  tree->Branch<int>("sensorID", &m_sensorID);
  tree->Branch<string>("ShapeName", &m_shapeName);
  tree->Branch<float>("ClusterEta", &m_clusterEta);
  tree->Branch<int>("uCellID", &m_uCellID);
  tree->Branch<int>("vCellID", &m_vCellID);
  tree->Branch<int>("signal", &m_signal);
  registerObject<TTree>(treename, tree);
}

void PXDGainCollectorModule::collect() // Do your event() stuff here
{
  // If no input, nothing to do
  if (!m_pxdCluster) return;

  string treename = string("tree");
  auto tree = getObjectPtr<TTree>(treename);

  for (auto& cluster :  m_pxdCluster) {

    // Apply cluster selection cuts
    if (cluster.getCharge() >= m_minClusterCharge && cluster.getSize() >= m_minClusterSize && cluster.getSize() < 10) {

      VxdID sensorID = cluster.getSensorID();
      const PXD::SensorInfo& Info = dynamic_cast<const PXD::SensorInfo&>(VXD::GeoCache::get(sensorID));

      // Sort all pixels related to the cluster
      set<PXD::Pixel> pixels;
      for (int i = 0; i < cluster.getSize(); i++) {
        const PXDDigit* const storeDigit = cluster.getRelationsTo<PXDDigit>("PXDDigits")[i];
        pixels.insert(PXD::Pixel(storeDigit, i));
      }

      // Fill the tree
      m_sensorID = int(sensorID);
      m_uCellID = Info.getUCellID(cluster.getU());
      m_vCellID = Info.getVCellID(cluster.getV());
      m_signal = cluster.getCharge();
      m_clusterEta = 1.0;  // i have no information about incidence angles
      m_shapeName = PXD::PXDClusterPositionEstimator::getInstance().getFullName(pixels, cluster.getUStart(), cluster.getVStart());
      tree->Fill();
    }
  }
}

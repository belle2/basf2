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
#include <pxd/reconstruction/PXDClusterPositionEstimator.h>
#include <pxd/geometry/SensorInfo.h>

#include <TTree.h>
#include <TMath.h>
#include <TH2I.h>

#include <boost/format.hpp>
#include <cmath>

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
  addParam("clusterKind", m_clusterKind, "Collect data for Clusterkind", int(0));
  addParam("binsU", m_binsU, "Number of bins for thetaU ", int(18));
  addParam("binsV", m_binsV, "Number of bins for thetaV ", int(18));
}

void PXDClusterPositionCollectorModule::prepare() // Do your initialise() stuff here
{
  m_pxdCluster.isRequired();

  // Data object creation --------------------------------------------------
  string gridname = str(format("GridKind_%1%") % m_clusterKind);
  auto grid = new TH2I(gridname.c_str(), gridname.c_str(),  m_binsU, -90.0, +90.0,  m_binsV, -90.0, +90.0);
  registerObject<TH2I>(gridname, grid);

  for (auto uBin = 1; uBin <= grid->GetXaxis()->GetNbins(); uBin++) {
    for (auto vBin = 1; vBin <= grid->GetYaxis()->GetNbins(); vBin++) {
      string treename = str(format("tree_%1%_%2%_%3%") % m_clusterKind % uBin % vBin);
      auto tree = new TTree(treename.c_str(), treename.c_str());
      tree->Branch<string>("ShapeName", &m_shapeName);
      tree->Branch<string>("MirroredShapeName", &m_mirroredShapeName);
      tree->Branch<float>("ClusterEta", &m_clusterEta);
      tree->Branch<float>("OffsetU", &m_positionOffsetU);
      tree->Branch<float>("OffsetV", &m_positionOffsetV);
      registerObject<TTree>(treename, tree);
    }
  }
}

void PXDClusterPositionCollectorModule::collect() // Do your event() stuff here
{
  // If no input, nothing to do
  if (!m_pxdCluster) return;

  string gridname = str(format("GridKind_%1%") % m_clusterKind);
  auto grid = getObjectPtr<TH2I>(gridname);

  for (auto& cluster :  m_pxdCluster) {

    // Ignore clustes with more than one truehit
    if (cluster.getRelationsTo<PXDTrueHit>().size() > 1) {
      continue;
    }

    // Ignore freak clusters with more than 1000 digits
    if (cluster.getSize() >= 1000) {
      continue;
    }

    // Ignore clusters with wrong clusterkind
    if (m_clusterKind != PXD::PXDClusterPositionEstimator::getInstance().getClusterkind(cluster)) {
      continue;
    }

    for (auto& truehit : cluster.getRelationsTo<PXDTrueHit>()) {

      auto mom = truehit.getMomentum();

      if (mom[2] == 0 ||  mom.Mag() < 0.02) continue;

      double thetaU = TMath::ATan2(mom[0] / mom[2], 1.0) * 180.0 / M_PI;
      double thetaV = TMath::ATan2(mom[1] / mom[2], 1.0) * 180.0 / M_PI;
      int uBin = grid->GetXaxis()->FindBin(thetaU);
      int vBin = grid->GetYaxis()->FindBin(thetaV);

      if (uBin < 1 || uBin > m_binsU || vBin < 1 || vBin > m_binsV) continue;

      // Add new entry to sources
      grid->Fill(thetaU, thetaV);

      VxdID sensorID = truehit.getSensorID();
      const PXD::SensorInfo& Info = dynamic_cast<const PXD::SensorInfo&>(VXD::GeoCache::get(sensorID));

      // Sort all pixels related to the cluster
      set<PXD::Pixel> pixels;
      for (int i = 0; i < cluster.getSize(); i++) {
        const PXDDigit* const storeDigit = cluster.getRelationsTo<PXDDigit>("PXDDigits")[i];
        pixels.insert(PXD::Pixel(storeDigit, i));
      }

      // Fill tree variables
      m_shapeName = PXD::PXDClusterPositionEstimator::getInstance().getShortName(pixels, cluster.getUStart(), cluster.getVStart(),
                    cluster.getVSize(), thetaU, thetaV);
      m_mirroredShapeName = PXD::PXDClusterPositionEstimator::getInstance().getMirroredShortName(pixels, cluster.getUStart(),
                            cluster.getVStart(), cluster.getVSize(), thetaU, thetaV);
      m_clusterEta = PXD::PXDClusterPositionEstimator::getInstance().computeEta(pixels, cluster.getVStart(), cluster.getVSize(), thetaU,
                     thetaV);
      m_positionOffsetU = truehit.getU() - Info.getUCellPosition(cluster.getUStart());
      m_positionOffsetV = truehit.getV() - Info.getVCellPosition(cluster.getVStart());

      // Fill the tree
      string treename = str(format("tree_%1%_%2%_%3%") % m_clusterKind % uBin % vBin);
      getObjectPtr<TTree>(treename)->Fill();
    }
  }
}

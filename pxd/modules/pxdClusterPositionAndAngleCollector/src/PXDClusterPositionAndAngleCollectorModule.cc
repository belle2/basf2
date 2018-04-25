/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Schwenker                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/modules/pxdClusterPositionAndAngleCollector/PXDClusterPositionAndAngleCollectorModule.h>
#include <pxd/dataobjects/PXDDigit.h>
#include <pxd/dataobjects/PXDTrueHit.h>
#include <mdst/dataobjects/MCParticle.h>
#include <vxd/geometry/GeoCache.h>
#include <pxd/reconstruction/PXDClusterPositionEstimator.h>
#include <pxd/geometry/SensorInfo.h>

#include <TTree.h>
#include <TMath.h>
#include <TH2F.h>

#include <boost/format.hpp>
#include <cmath>

using namespace std;
using boost::format;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PXDClusterPositionAndAngleCollector)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PXDClusterPositionAndAngleCollectorModule::PXDClusterPositionAndAngleCollectorModule() : CalibrationCollectorModule()
{
  // Set module properties
  setDescription("Calibration collector module for PXD cluster position estimation");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("clustersName", m_storeClustersName, "Name of the collection to use for PXDClusters", string(""));
  addParam("clusterKind", m_clusterKind, "Collect data for Clusterkind", int(0));
  addParam("binsU", m_binsU, "Number of bins for thetaU ", int(2));
  addParam("binsV", m_binsV, "Number of bins for thetaV ", int(2));
}

void PXDClusterPositionAndAngleCollectorModule::prepare() // Do your initialise() stuff here
{
  m_pxdCluster.isRequired();

  // Data object creation --------------------------------------------------
  string gridname = str(format("GridKind_%1%") % m_clusterKind);
  auto grid = new TH2F(gridname.c_str(), gridname.c_str(),  m_binsU, -90.0, +90.0,  m_binsV, -90.0, +90.0);
  registerObject<TH2F>(gridname, grid);

  string pitchtreename = "pitchtree";
  auto ptree = new TTree(pitchtreename.c_str(), pitchtreename.c_str());
  ptree->Branch<int>("ClusterKind", &m_clusterKind);
  ptree->Branch<float>("PitchV", &m_pitchV);
  registerObject<TTree>(pitchtreename, ptree);

  for (auto uBin = 1; uBin <= grid->GetXaxis()->GetNbins(); uBin++) {
    for (auto vBin = 1; vBin <= grid->GetYaxis()->GetNbins(); vBin++) {
      string treename = str(format("tree_%1%_%2%_%3%") % m_clusterKind % uBin % vBin);
      auto tree = new TTree(treename.c_str(), treename.c_str());
      tree->Branch<string>("ShapeName", &m_shapeName);
      tree->Branch<string>("MirroredShapeName", &m_mirroredShapeName);
      tree->Branch<float>("ClusterEta", &m_clusterEta);
      tree->Branch<float>("ClusterCharge", & m_clusterCharge);
      tree->Branch<float>("OffsetU", &m_positionOffsetU);
      tree->Branch<float>("OffsetV", &m_positionOffsetV);
      tree->Branch<float>("ThetaU", &m_thetaU);
      tree->Branch<float>("ThetaV", &m_thetaV);
      tree->Branch<float>("MomentumMag", &m_momentumMag);
      tree->Branch<int>("SizeV", &m_sizeV);
      registerObject<TTree>(treename, tree);
    }
  }
}

void PXDClusterPositionAndAngleCollectorModule::collect() // Do your event() stuff here
{
  // If no input, nothing to do
  if (!m_pxdCluster) return;

  string gridname = str(format("GridKind_%1%") % m_clusterKind);
  auto grid = getObjectPtr<TH2F>(gridname);

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

    // For setting of vertex close to surface of for PXD kinds of pixels set:
    // Kind 0:  sensor 1.3.2
    // Kind 1:  sensor 1.3.2
    // Kind 2:  sensor 2.4.2
    // Kind 3:  sensor 2.4.2

    VxdID sensorID = cluster.getSensorID();
    if (sensorID != VxdID("1.3.2")) {
      continue;
    }

    for (auto& truehit : cluster.getRelationsTo<PXDTrueHit>()) {

      // Default pdg code, in case branch is MCParticels missing
      int pdg = 0;
      for (auto& mcp : truehit.getRelationsFrom<MCParticle>()) {
        pdg = mcp.getPDG();
        // Only look at primary particles
        if (not mcp.hasStatus(1))
          continue;
      }

      auto mom = truehit.getMomentum();
      if (mom[2] == 0 ||  mom.Mag() < 0.02) continue;

      // From here, we have a valid truehit-cluster pair.

      // The incidence angles of the truehit
      double thetaU = TMath::ATan2(mom[0] / mom[2], 1.0) * 180.0 / M_PI;
      double thetaV = TMath::ATan2(mom[1] / mom[2], 1.0) * 180.0 / M_PI;

      // Fill true hit angles into hitmap
      grid->Fill(thetaU, thetaV);

      const PXD::SensorInfo& Info = dynamic_cast<const PXD::SensorInfo&>(VXD::GeoCache::get(sensorID));

      // Sort all pixels related to the cluster
      set<PXD::Pixel> pixels;
      for (int i = 0; i < cluster.getSize(); i++) {
        const PXDDigit* const storeDigit = cluster.getRelationsTo<PXDDigit>("PXDDigits")[i];
        pixels.insert(PXD::Pixel(storeDigit, i));
      }

      // The mapping from cluster shape to incidence angles is ambiguous, so we fill data from
      // this pair into four angle bins which represent the four angle sectors (++,+-,-+,--).
      for (auto uBin = 1; uBin <= grid->GetXaxis()->GetNbins(); uBin++) {
        for (auto vBin = 1; vBin <= grid->GetYaxis()->GetNbins(); vBin++) {

          // Bin is centered around angles
          auto binThetaU = grid->GetXaxis()->GetBinCenter(uBin);
          auto binThetaV = grid->GetYaxis()->GetBinCenter(vBin);

          // Fill tree variables
          m_shapeName = PXD::PXDClusterPositionEstimator::getInstance().getShortName(pixels, cluster.getUStart(), cluster.getVStart(),
                        cluster.getVSize(), binThetaU, binThetaV);
          m_mirroredShapeName = "DUMMY";
          m_clusterEta = PXD::PXDClusterPositionEstimator::getInstance().computeEta(pixels, cluster.getVStart(), cluster.getVSize(),
                         binThetaU, binThetaV);
          m_clusterCharge = cluster.getCharge();
          m_positionOffsetU = truehit.getU() - Info.getUCellPosition(cluster.getUStart());
          m_positionOffsetV = truehit.getV() - Info.getVCellPosition(cluster.getVStart());
          m_thetaU = thetaU;
          m_thetaV = thetaV;
          m_momentumMag = mom.Mag();
          m_pitchV = Info.getVPitch(truehit.getV());
          m_sizeV = cluster.getVSize();

          // Fill the tree
          string treename = str(format("tree_%1%_%2%_%3%") % m_clusterKind % uBin % vBin);
          getObjectPtr<TTree>(treename)->Fill();
        }
      }

      // Fill the pitch tree (this should happen only once per collector)
      string pitchtreename = "pitchtree";
      if (getObjectPtr<TTree>(pitchtreename)->GetEntries() == 0) {
        getObjectPtr<TTree>(pitchtreename)->Fill();
      }
    }
  }
}

/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <pxd/modules/pxdClusterChargeCollector/PXDClusterChargeCollectorModule.h>

#include <framework/datastore/RelationArray.h>

#include <vxd/geometry/GeoCache.h>
#include <pxd/geometry/SensorInfo.h>
#include <pxd/reconstruction/PXDGainCalibrator.h>

#include <TTree.h>
#include <TH2I.h>

#include <boost/format.hpp>

using namespace std;
using boost::format;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PXDClusterChargeCollector)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PXDClusterChargeCollectorModule::PXDClusterChargeCollectorModule() : CalibrationCollectorModule()
  , m_signal(0), m_run(0), m_exp(0)
{
  // Set module properties
  setDescription("Calibration collector module for cluster charge related calibrations.");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("clustersName", m_storeClustersName, "Name of the collection to use for PXDClusters", string(""));
  addParam("mcParticlesName", m_storeMCParticlesName, "Name of the collection to use for MCParticles", string("MCParticles"));
  addParam("minClusterCharge", m_minClusterCharge, "Minimum cluster charge cut", int(0));
  addParam("minClusterSize", m_minClusterSize, "Minimum cluster size cut ", int(2));
  addParam("maxClusterSize", m_maxClusterSize, "Maximum cluster size cut ", int(6));
  addParam("nBinsU", m_nBinsU, "Number of gain corrections per sensor along u side", int(4));
  addParam("nBinsV", m_nBinsV, "Number of gain corrections per sensor along v side", int(6));
  addParam("chargePayloadName", m_chargeName, "Payload name for Cluster Charge to be read from DB", string(""));
  addParam("gainPayloadName", m_gainName, "Payload name for Gain to be read from DB", string(""));
  addParam("fillChargeHistogram", m_fillChargeHistogram, "Flag to fill Charge histograms", bool(false));
  addParam("matchTrack", m_matchTrack,
           "Flag to use track matched clusters (=1) and apply theta angle projection to cluster charge (=2)", int(0));
  addParam("mcSamples", m_mcSamples, "Flag to deal with MC samples", bool(false));
  addParam("relationCheck", m_relationCheck, "Flag to check relations between PXDClusters and PXDClustersFromTracks", bool(false));

}

void PXDClusterChargeCollectorModule::prepare() // Do your initialise() stuff here
{
  m_pxdClusters.isRequired(m_storeClustersName);
  if (m_relationCheck) {
    std::string storeClustersName2;
    storeClustersName2 = (m_storeClustersName == "PXDClusters") ?
                         "PXDClustersFromTracks" : "PXDClusters";
    StoreArray<PXDCluster> pxdClusters2;
    pxdClusters2.isRequired(storeClustersName2);
  }
  m_tracks.isOptional(); //m_storeTracksName);
  m_recoTracks.isOptional(); //m_storeRecoTracksName);

  if (m_mcSamples && m_matchTrack > 0) {
    m_mcParticles.isRequired(m_storeMCParticlesName);
  } else {
    m_mcParticles.isOptional();
  }

  if (m_nBinsU == 0) {
    B2WARNING("Number of bins along u side incremented from 0->1");
    m_nBinsU = 1;
  }

  if (m_nBinsV == 0) {
    B2WARNING("Number of bins along v side incremented from 0->1");
    m_nBinsV = 1;
  }

  auto gTools = VXD::GeoCache::getInstance().getGeoTools();
  int nPXDSensors = gTools->getNumberOfPXDSensors();

  //-------------------------------------------------------------------------------------
  // PXDClusterCounter: Count the number of PXDClusters and store charge for each uBin/vBin pair
  //-------------------------------------------------------------------------------------

  auto hPXDClusterCounter = new TH1I("hPXDClusterCounter", "Number of clusters found in data sample",
                                     m_nBinsU * m_nBinsV * nPXDSensors, 0,
                                     m_nBinsU * m_nBinsV * nPXDSensors);
  hPXDClusterCounter->GetXaxis()->SetTitle("bin id");
  hPXDClusterCounter->GetYaxis()->SetTitle("Number of clusters");
  for (int iSensor = 0; iSensor < nPXDSensors; iSensor++) {
    for (int uBin = 0; uBin < m_nBinsU; uBin++) {
      for (int vBin = 0; vBin < m_nBinsV; vBin++) {
        VxdID id = gTools->getSensorIDFromPXDIndex(iSensor);
        string sensorDescr = id;
        hPXDClusterCounter->GetXaxis()->SetBinLabel(iSensor * m_nBinsU * m_nBinsV + uBin * m_nBinsV + vBin + 1,
                                                    str(format("%1%_%2%_%3%") % sensorDescr % uBin % vBin).c_str());
      }
    }
  }
  registerObject<TH1I>("PXDClusterCounter", hPXDClusterCounter);
  if (m_fillChargeHistogram) {
    auto hPXDClusterCharge = new TH2I("hPXDClusterCharge", "Charge of clusters found in data sample",
                                      m_nBinsU * m_nBinsV * nPXDSensors, 0,
                                      m_nBinsU * m_nBinsV * nPXDSensors,
                                      250, 0, 250);
    hPXDClusterCharge->GetXaxis()->SetTitle("bin id");
    hPXDClusterCharge->GetYaxis()->SetTitle("Cluster charge [ADU]");
    registerObject<TH2I>("PXDClusterCharge", hPXDClusterCharge);
  }

  //----------------------------------------------------------------------
  // PXDTrees: One tree to store the calibration data for each grid bin
  //----------------------------------------------------------------------

  for (int iSensor = 0; iSensor < nPXDSensors; iSensor++) {
    for (int uBin = 0; uBin < m_nBinsU; uBin++) {
      for (int vBin = 0; vBin < m_nBinsV; vBin++) {
        VxdID id = gTools->getSensorIDFromPXDIndex(iSensor);
        auto layerNumber = id.getLayerNumber();
        auto ladderNumber = id.getLadderNumber();
        auto sensorNumber = id.getSensorNumber();
        string treename = str(format("tree_%1%_%2%_%3%_%4%_%5%") % layerNumber % ladderNumber % sensorNumber % uBin % vBin);
        auto tree = new TTree(treename.c_str(), treename.c_str());
        tree->Branch<int>("signal", &m_signal);
        registerObject<TTree>(treename, tree);
      }
    }
  }

  auto dbtree = new TTree("dbtree", "dbtree");
  dbtree->Branch<int>("run", &m_run);
  dbtree->Branch<int>("exp", &m_exp);
  dbtree->Branch<PXDClusterChargeMapPar>("chargeMap", &m_chargeMap);
  dbtree->Branch<PXDGainMapPar>("gainMap", &m_gainMap);
  registerObject<TTree>("dbtree", dbtree);
}

void PXDClusterChargeCollectorModule::startRun() // Do your beginRun() stuff here
{
  m_run = m_evtMetaData->getRun();
  m_exp = m_evtMetaData->getExperiment();
  if (m_chargeName.length()) {
    DBObjPtr<PXDClusterChargeMapPar> chargeMap(m_chargeName);
    m_chargeMap = *chargeMap;
  } else {
    m_chargeMap = PXDClusterChargeMapPar();
  }
  if (m_gainName.length()) {
    DBObjPtr<PXDGainMapPar> gainMap(m_gainName);
    m_gainMap = *gainMap;
  } else {
    m_gainMap = PXDGainMapPar();
  }
  getObjectPtr<TTree>("dbtree")->Fill();
}

void PXDClusterChargeCollectorModule::collect() // Do your event() stuff here
{
  // If no input, nothing to do
  if (!m_pxdClusters) return;

  auto gTools = VXD::GeoCache::getInstance().getGeoTools();

  if (!m_matchTrack || m_mcSamples) { // all clusters for data

    for (auto& cluster :  m_pxdClusters) {
      if (m_relationCheck) {
        if (m_storeClustersName == "PXDClusters") {
          RelationVector<PXDCluster> relPXDClusters = cluster.getRelationsTo<PXDCluster>("PXDClustersFromTracks");
          if (!relPXDClusters.size()) continue;
        } else if (m_storeClustersName == "PXDClustersFromTracks") {
          RelationVector<PXDCluster> relPXDClusters = DataStore::getRelationsWithObj<PXDCluster>(&cluster, "PXDClusters");
          if (!relPXDClusters.size()) continue;
        }
      }
      double correction = 1.0;
      if (m_matchTrack > 0) { // mc samples
        RelationVector<MCParticle> mcParticlesPXD = DataStore::getRelationsWithObj<MCParticle>(&cluster, m_storeMCParticlesName);
        if (!mcParticlesPXD.size()) continue;
        if (m_matchTrack == 2) {
          correction = sin(mcParticlesPXD[0]->getMomentum().Theta());
        }
      }
      // Apply cluster selection cuts
      if (cluster.getCharge() >= m_minClusterCharge && cluster.getSize() >= m_minClusterSize && cluster.getSize() <= m_maxClusterSize) {

        VxdID sensorID = cluster.getSensorID();
        const PXD::SensorInfo& Info = dynamic_cast<const PXD::SensorInfo&>(VXD::GeoCache::get(sensorID));
        auto uID = Info.getUCellID(cluster.getU());
        auto vID = Info.getVCellID(cluster.getV());
        auto iSensor = gTools->getPXDSensorIndex(sensorID);
        auto layerNumber = sensorID.getLayerNumber();
        auto ladderNumber = sensorID.getLadderNumber();
        auto sensorNumber = sensorID.getSensorNumber();
        auto uBin = PXD::PXDGainCalibrator::getInstance().getBinU(sensorID, uID, vID, m_nBinsU);
        auto vBin = PXD::PXDGainCalibrator::getInstance().getBinV(sensorID, vID, m_nBinsV);
        string treename = str(format("tree_%1%_%2%_%3%_%4%_%5%") % layerNumber % ladderNumber % sensorNumber % uBin % vBin);

        // Compute variables from cluster needed for gain estimation
        m_signal = cluster.getCharge();
        // Fill variabels into tree
        getObjectPtr<TTree>(treename)->Fill();
        // Increment the counter & store charge (optional)
        getObjectPtr<TH1I>("PXDClusterCounter")->Fill(iSensor * m_nBinsU * m_nBinsV + uBin * m_nBinsV + vBin);
        if (m_fillChargeHistogram)
          getObjectPtr<TH2I>("PXDClusterCharge")->Fill(iSensor * m_nBinsU * m_nBinsV + uBin * m_nBinsV + vBin,
                                                       cluster.getCharge()*correction);
      }

    }

  } else { // store only clusters matched with tracks

    // If no input, nothing to do
    if (!m_tracks) return;

    for (const Track& track : m_tracks) {

      RelationVector<RecoTrack> recoTrack = track.getRelationsTo<RecoTrack>(m_storeRecoTracksName);
      if (!recoTrack.size()) continue;
      RelationVector<PXDCluster> pxdClustersTrack = DataStore::getRelationsWithObj<PXDCluster>(recoTrack[0], m_storeClustersName);

      const TrackFitResult* tfr = track.getTrackFitResultWithClosestMass(Const::pion);
      double correction = 1.0;
      if (tfr && m_matchTrack == 2) correction = sin(tfr->getMomentum().Theta());

      for (auto& cluster : pxdClustersTrack) {
        if (m_relationCheck) {
          if (m_storeClustersName == "PXDClustersFromTracks") {
            RelationVector<PXDCluster> relPXDClusters = DataStore::getRelationsWithObj<PXDCluster>(&cluster, "PXDClusters");
            if (!relPXDClusters.size()) continue;
          } else {
            B2WARNING("Relation check for data only works when clustersName is set to PXDClustersFromTracks");
          }
        }

        // Apply cluster selection cuts
        if (cluster.getCharge() >= m_minClusterCharge && cluster.getSize() >= m_minClusterSize && cluster.getSize() <= m_maxClusterSize) {

          VxdID sensorID = cluster.getSensorID();
          const PXD::SensorInfo& Info = dynamic_cast<const PXD::SensorInfo&>(VXD::GeoCache::get(sensorID));
          auto uID = Info.getUCellID(cluster.getU());
          auto vID = Info.getVCellID(cluster.getV());
          auto iSensor = gTools->getPXDSensorIndex(sensorID);
          auto layerNumber = sensorID.getLayerNumber();
          auto ladderNumber = sensorID.getLadderNumber();
          auto sensorNumber = sensorID.getSensorNumber();
          auto uBin = PXD::PXDGainCalibrator::getInstance().getBinU(sensorID, uID, vID, m_nBinsU);
          auto vBin = PXD::PXDGainCalibrator::getInstance().getBinV(sensorID, vID, m_nBinsV);
          string treename = str(format("tree_%1%_%2%_%3%_%4%_%5%") % layerNumber % ladderNumber % sensorNumber % uBin % vBin);

          // Compute variables from cluster needed for gain estimation
          m_signal = cluster.getCharge();
          // Fill variabels into tree
          getObjectPtr<TTree>(treename)->Fill();
          // Increment the counter & store charge (optional)
          getObjectPtr<TH1I>("PXDClusterCounter")->Fill(iSensor * m_nBinsU * m_nBinsV + uBin * m_nBinsV + vBin);
          if (m_fillChargeHistogram)
            getObjectPtr<TH2I>("PXDClusterCharge")->Fill(iSensor * m_nBinsU * m_nBinsV + uBin * m_nBinsV + vBin,
                                                         cluster.getCharge()*correction);

        }

      }

    }

  }

}

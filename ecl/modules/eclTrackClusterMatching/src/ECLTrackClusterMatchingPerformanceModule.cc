/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Frank Meier                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <ecl/modules/eclTrackClusterMatching/ECLTrackClusterMatchingPerformanceModule.h>

#include <framework/datastore/RelationVector.h>
#include <mdst/dataobjects/HitPatternCDC.h>
#include <mdst/dataobjects/HitPatternVXD.h>
#include <framework/gearbox/Const.h>

#include <root/TFile.h>
#include <root/TTree.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ECLTrackClusterMatchingPerformance)

ECLTrackClusterMatchingPerformanceModule::ECLTrackClusterMatchingPerformanceModule() :
  Module(), m_trackProperties()
{
  setDescription("Module to test the track cluster matching efficiency. Writes information about the tracks and MCParticles in a ROOT file.");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("outputFileName", m_outputFileName, "Name of output root file.",
           std::string("ECLTrackClusterMatchingPerformance.root"));
  addParam("minClusterEnergy", m_minClusterEnergy, "Minimal cluster energy in units of particle's true energy for MC match",
           0.5);
  addParam("minWeight", m_minWeight, "Fraction of cluster energy required to originate from particle to be matched to cluster.",
           0.5);
  addParam("trackClusterRelationName", m_trackClusterRelationName, "Name of relation array between tracks and ECL clusters",
           std::string(""));
}

void ECLTrackClusterMatchingPerformanceModule::initialize()
{
  // MCParticles and Tracks needed for this module
  m_mcParticles.isRequired();
  m_tracks.isRequired();
  m_trackFitResults.isRequired();
  m_eclClusters.isRequired();

  m_outputFile = new TFile(m_outputFileName.c_str(), "RECREATE");
  TDirectory* oldDir = gDirectory;
  m_outputFile->cd();
  m_tracksTree = new TTree("tracks", "tracks");
  m_clusterTree = new TTree("cluster", "cluster");
  oldDir->cd();

  setupTree();
}

void ECLTrackClusterMatchingPerformanceModule::event()
{
  m_iEvent = m_EventMetaData->getEvent();
  m_iRun = m_EventMetaData->getRun();
  m_iExperiment = m_EventMetaData->getExperiment();

  for (const ECLCluster& eclCluster : m_eclClusters) {
    setClusterVariablesToDefaultValue();
    bool found_photon = false, found_mcmatch = false, found_charged_stable = false;
    // find all MCParticles matched to the ECLCluster
    const auto& relatedMCParticles = eclCluster.getRelationsWith<MCParticle>();
    for (unsigned int index = 0; index < relatedMCParticles.size() && (!found_photon || !found_charged_stable); ++index) {
      const MCParticle* relatedMCParticle = relatedMCParticles.object(index);
      // check if matched MCParticle is primary photon
      if (!(isPrimaryMcParticle(*relatedMCParticle))) continue;
      found_mcmatch = true;
      // get total energy of MCParticle deposited in this ECLCluster
      const double weight = relatedMCParticles.weight(index);
      // check that at least 50% of the generated energy of the particle is contained in this ECLCluster
      // and check that the total cluster energy is greater than 50% of the energy coming from the particle

      ECLCluster::EHypothesisBit hypo = ECLCluster::EHypothesisBit::c_nPhotons;
      if (eclCluster.hasHypothesis(ECLCluster::EHypothesisBit::c_neutralHadron)
          and not eclCluster.hasHypothesis(ECLCluster::EHypothesisBit::c_nPhotons)) {
        hypo = ECLCluster::EHypothesisBit::c_neutralHadron;
      }

      if (eclCluster.getEnergy(hypo) >= 0.5 * relatedMCParticle->getEnergy()) {
        if (isChargedStable(*relatedMCParticle) && weight >= 0.5 * relatedMCParticle->getEnergy()) {
          found_charged_stable = true;
        } else if (relatedMCParticle->getPDG() == Const::photon.getPDGCode() && weight >= 0.5 * eclCluster.getEnergy(hypo)
                   && !found_photon) {
          found_photon = true;
          m_photonEnergy = relatedMCParticle->getEnergy();
        }
      }
    }
    if (found_mcmatch) {
      if (eclCluster.isTrack()) {
        m_clusterIsTrack = 1;
      }
      m_clusterIsPhoton = int(found_photon);
      m_clusterIsChargedStable = int(found_charged_stable);
      m_clusterPhi = eclCluster.getPhi();
      m_clusterTheta = eclCluster.getTheta();
      if (eclCluster.hasHypothesis(ECLCluster::EHypothesisBit::c_nPhotons)) {
        if (eclCluster.hasHypothesis(ECLCluster::EHypothesisBit::c_neutralHadron)) {
          m_clusterHypothesis = 56;
        } else {
          m_clusterHypothesis = 5;
        }
        m_clusterEnergy = eclCluster.getEnergy(ECLCluster::EHypothesisBit::c_nPhotons);
      } else if (eclCluster.hasHypothesis(ECLCluster::EHypothesisBit::c_neutralHadron)) {
        m_clusterHypothesis = 6;
        m_clusterEnergy = eclCluster.getEnergy(ECLCluster::EHypothesisBit::c_neutralHadron);
      }

      m_clusterErrorTiming = eclCluster.getDeltaTime99();
      m_clusterE1E9 = eclCluster.getE1oE9();
      m_clusterDetectorRegion = eclCluster.getDetectorRegion();
      m_clusterMinTrkDistance = eclCluster.getMinTrkDistance();
      m_clusterDeltaL = eclCluster.getDeltaL();
      m_clusterTree->Fill();
    }
  }

  for (const MCParticle& mcParticle : m_mcParticles) {
    // check status of mcParticle
    if (isPrimaryMcParticle(mcParticle) && isChargedStable(mcParticle) && mcParticle.hasStatus(MCParticle::c_StableInGenerator)) {
      setVariablesToDefaultValue();

      int pdgCode = mcParticle.getPDG();
      B2DEBUG(29, "Primary MCParticle has PDG code " << pdgCode);
      m_trackProperties.pdg_gen = pdgCode;

      m_trackProperties.cosTheta_gen = mcParticle.getMomentum().CosTheta();
      m_trackProperties.phi_gen = mcParticle.getMomentum().Phi();
      m_trackProperties.ptot_gen = mcParticle.getMomentum().Mag();
      m_trackProperties.pt_gen = mcParticle.getMomentum().Pt();
      m_trackProperties.px_gen = mcParticle.getMomentum().Px();
      m_trackProperties.py_gen = mcParticle.getMomentum().Py();
      m_trackProperties.pz_gen = mcParticle.getMomentum().Pz();
      m_trackProperties.x_gen = mcParticle.getVertex().X();
      m_trackProperties.y_gen = mcParticle.getVertex().Y();
      m_trackProperties.z_gen = mcParticle.getVertex().Z();

      const Track* b2Track = nullptr;
      double maximumWeight = -2;
      // find highest rated Track
      const auto& relatedTracks = mcParticle.getRelationsWith<Track>();
      for (unsigned int index = 0; index < relatedTracks.size(); ++index) {
        const Track* relatedTrack = relatedTracks.object(index);
        const double weight = relatedTracks.weight(index);

        if (weight > maximumWeight) {
          maximumWeight = weight;
          b2Track = relatedTrack;
        }
      }

      if (b2Track) {
        const TrackFitResult* fitResult = b2Track->getTrackFitResultWithClosestMass(Const::ChargedStable(std::abs(pdgCode)));
        B2ASSERT("Related Belle2 Track has no related track fit result!", fitResult);

        // write some data to the root tree
        TVector3 mom = fitResult->getMomentum();
        m_trackProperties.cosTheta = mom.CosTheta();
        m_trackProperties.phi = mom.Phi();
        m_trackProperties.ptot = mom.Mag();
        m_trackProperties.pt = mom.Pt();
        m_trackProperties.px = mom.Px();
        m_trackProperties.py = mom.Py();
        m_trackProperties.pz = mom.Pz();
        m_trackProperties.x = fitResult->getPosition().X();
        m_trackProperties.y = fitResult->getPosition().Y();
        m_trackProperties.z = fitResult->getPosition().Z();

        m_pValue = fitResult->getPValue();
        m_charge = (int)fitResult->getChargeSign();
        m_d0 = fitResult->getD0();
        m_z0 = fitResult->getZ0();

        // Count hits
        m_trackProperties.nPXDhits = fitResult->getHitPatternVXD().getNPXDHits();
        m_trackProperties.nSVDhits = fitResult->getHitPatternVXD().getNSVDHits();
        m_trackProperties.nCDChits = fitResult->getHitPatternCDC().getNHits();

        m_lastCDCLayer = fitResult->getHitPatternCDC().getLastLayer();

        const ECLCluster* eclCluster_PhotonHypothesis_track_related = nullptr;
        const ECLCluster* eclCluster_HadronHypothesis_track_related = nullptr;
        for (const auto& eclCluster : b2Track->getRelationsTo<ECLCluster>("", m_trackClusterRelationName)) {
          if (!(eclCluster.isTrack())) continue;
          if (eclCluster.hasHypothesis(ECLCluster::EHypothesisBit::c_nPhotons)) {
            m_matchedToPhotonHypothesisECLCluster = 1;
            m_matchedPhotonHypothesisClusterDetectorRegion = eclCluster.getDetectorRegion();
            m_matchedPhotonHypothesisClusterTheta = eclCluster.getTheta();
            m_matchedPhotonHypothesisClusterPhi = eclCluster.getPhi();
            m_matchedPhotonHypothesisClusterMinTrkDistance = eclCluster.getMinTrkDistance();
            m_matchedPhotonHypothesisClusterDeltaL = eclCluster.getDeltaL();
            eclCluster_PhotonHypothesis_track_related = &eclCluster;
          }
          if (eclCluster.hasHypothesis(ECLCluster::EHypothesisBit::c_neutralHadron)) {
            m_matchedToHadronHypothesisECLCluster = 1;
            m_matchedHadronHypothesisClusterDetectorRegion = eclCluster.getDetectorRegion();
            m_matchedHadronHypothesisClusterTheta = eclCluster.getTheta();
            m_matchedHadronHypothesisClusterPhi = eclCluster.getPhi();
            m_matchedHadronHypothesisClusterMinTrkDistance = eclCluster.getMinTrkDistance();
            m_matchedHadronHypothesisClusterDeltaL = eclCluster.getDeltaL();
            eclCluster_HadronHypothesis_track_related = &eclCluster;
          }
        }
        // find ECLCluster in which the particle has deposited the majority of its energy
        maximumWeight = -2.;
        const ECLCluster* eclCluster_matchedBestToMCParticle = nullptr;
        const auto& relatedECLClusters = mcParticle.getRelationsFrom<ECLCluster>();
        for (unsigned int index = 0; index < relatedECLClusters.size(); ++index) {
          const ECLCluster* relatedECLCluster = relatedECLClusters.object(index);
          if (!relatedECLCluster->hasHypothesis(ECLCluster::EHypothesisBit::c_nPhotons)) continue;
          const double weight = relatedECLClusters.weight(index);
          if (weight > maximumWeight) {
            eclCluster_matchedBestToMCParticle = relatedECLCluster;
            maximumWeight = weight;
          }
        }
        if (eclCluster_matchedBestToMCParticle != nullptr) {
          m_mcparticle_cluster_match = 1;
          m_mcparticle_cluster_energy = maximumWeight;
          m_mcparticle_cluster_theta = eclCluster_matchedBestToMCParticle->getTheta();
          m_mcparticle_cluster_phi = eclCluster_matchedBestToMCParticle->getPhi();
          m_mcparticle_cluster_detectorregion = eclCluster_matchedBestToMCParticle->getDetectorRegion();
          if ((eclCluster_PhotonHypothesis_track_related == eclCluster_matchedBestToMCParticle
               && eclCluster_matchedBestToMCParticle->hasHypothesis(ECLCluster::EHypothesisBit::c_nPhotons))
              || (eclCluster_HadronHypothesis_track_related == eclCluster_matchedBestToMCParticle
                  && eclCluster_matchedBestToMCParticle->hasHypothesis(ECLCluster::EHypothesisBit::c_neutralHadron))) {
            m_sameclusters = 1;
          }
        }
      }
      m_tracksTree->Fill(); // write data to tree
    }
  }
}


void ECLTrackClusterMatchingPerformanceModule::terminate()
{
  writeData();
}

bool ECLTrackClusterMatchingPerformanceModule::isPrimaryMcParticle(const MCParticle& mcParticle)
{
  return mcParticle.hasStatus(MCParticle::c_PrimaryParticle);
}

bool ECLTrackClusterMatchingPerformanceModule::isChargedStable(const MCParticle& mcParticle)
{
  return Const::chargedStableSet.find(abs(mcParticle.getPDG()))
         != Const::invalidParticle;
}

void ECLTrackClusterMatchingPerformanceModule::setupTree()
{
  if (m_tracksTree == nullptr || m_clusterTree == nullptr) {
    B2FATAL("Data tree or event tree was not created.");
  }
  addVariableToTree("expNo", m_iExperiment, m_tracksTree);
  addVariableToTree("runNo", m_iRun, m_tracksTree);
  addVariableToTree("evtNo", m_iEvent, m_tracksTree);

  addVariableToTree("pdgCode", m_trackProperties.pdg_gen, m_tracksTree);

  addVariableToTree("cosTheta", m_trackProperties.cosTheta, m_tracksTree);
  addVariableToTree("cosTheta_gen",  m_trackProperties.cosTheta_gen, m_tracksTree);

  addVariableToTree("phi", m_trackProperties.phi, m_tracksTree);
  addVariableToTree("phi_gen", m_trackProperties.phi_gen, m_tracksTree);

  addVariableToTree("px", m_trackProperties.px, m_tracksTree);
  addVariableToTree("px_gen", m_trackProperties.px_gen, m_tracksTree);

  addVariableToTree("py", m_trackProperties.py, m_tracksTree);
  addVariableToTree("py_gen", m_trackProperties.py_gen, m_tracksTree);

  addVariableToTree("pz", m_trackProperties.pz, m_tracksTree);
  addVariableToTree("pz_gen", m_trackProperties.pz_gen, m_tracksTree);

  addVariableToTree("x", m_trackProperties.x, m_tracksTree);
  addVariableToTree("x_gen", m_trackProperties.x_gen, m_tracksTree);

  addVariableToTree("y", m_trackProperties.y, m_tracksTree);
  addVariableToTree("y_gen", m_trackProperties.y_gen, m_tracksTree);

  addVariableToTree("z", m_trackProperties.z, m_tracksTree);
  addVariableToTree("z_gen", m_trackProperties.z_gen, m_tracksTree);

  addVariableToTree("pt", m_trackProperties.pt, m_tracksTree);
  addVariableToTree("pt_gen", m_trackProperties.pt_gen, m_tracksTree);

  addVariableToTree("ptot", m_trackProperties.ptot, m_tracksTree);
  addVariableToTree("ptot_gen", m_trackProperties.ptot_gen, m_tracksTree);

  addVariableToTree("pValue", m_pValue, m_tracksTree);

  addVariableToTree("charge", m_charge, m_tracksTree);

  addVariableToTree("d0", m_d0, m_tracksTree);
  addVariableToTree("z0", m_z0, m_tracksTree);

  addVariableToTree("nPXDhits", m_trackProperties.nPXDhits, m_tracksTree);
  addVariableToTree("nSVDhits", m_trackProperties.nSVDhits, m_tracksTree);
  addVariableToTree("nCDChits", m_trackProperties.nCDChits, m_tracksTree);

  addVariableToTree("lastCDCLayer", m_lastCDCLayer, m_tracksTree);

  addVariableToTree("ECLMatchPhotonHypothesis", m_matchedToPhotonHypothesisECLCluster, m_tracksTree);
  addVariableToTree("ECLMatchHadronHypothesis", m_matchedToHadronHypothesisECLCluster, m_tracksTree);
  addVariableToTree("MCParticleClusterMatch", m_mcparticle_cluster_match, m_tracksTree);

  addVariableToTree("SameMatch", m_sameclusters, m_tracksTree);

  addVariableToTree("PhotonHypothesisDetectorRegion", m_matchedPhotonHypothesisClusterDetectorRegion, m_tracksTree);
  addVariableToTree("PhotonHypothesisClusterTheta", m_matchedPhotonHypothesisClusterTheta, m_tracksTree);
  addVariableToTree("PhotonHypothesisClusterPhi", m_matchedPhotonHypothesisClusterPhi, m_tracksTree);
  addVariableToTree("PhotonHypothesisMinTrkDistance", m_matchedPhotonHypothesisClusterMinTrkDistance, m_tracksTree);
  addVariableToTree("PhotonHypothesisDeltaL", m_matchedPhotonHypothesisClusterDeltaL, m_tracksTree);

  addVariableToTree("HadronHypothesisDetectorRegion", m_matchedHadronHypothesisClusterDetectorRegion, m_tracksTree);
  addVariableToTree("HadronHypothesisClusterTheta", m_matchedHadronHypothesisClusterTheta, m_tracksTree);
  addVariableToTree("HadronHypothesisClusterPhi", m_matchedHadronHypothesisClusterPhi, m_tracksTree);
  addVariableToTree("HadronHypothesisMinTrkDistance", m_matchedHadronHypothesisClusterMinTrkDistance, m_tracksTree);
  addVariableToTree("HadronHypothesisDeltaL", m_matchedHadronHypothesisClusterDeltaL, m_tracksTree);

  addVariableToTree("MCClusterEnergy", m_mcparticle_cluster_energy, m_tracksTree);
  addVariableToTree("MCClusterDetectorRegion", m_mcparticle_cluster_detectorregion, m_tracksTree);
  addVariableToTree("MCClusterTheta", m_mcparticle_cluster_theta, m_tracksTree);
  addVariableToTree("MCClusterPhi", m_mcparticle_cluster_phi, m_tracksTree);


  addVariableToTree("expNo", m_iExperiment, m_clusterTree);
  addVariableToTree("runNo", m_iRun, m_clusterTree);
  addVariableToTree("evtNo", m_iEvent, m_clusterTree);

  addVariableToTree("ClusterPhi", m_clusterPhi, m_clusterTree);
  addVariableToTree("ClusterTheta", m_clusterTheta, m_clusterTree);
  addVariableToTree("ClusterHypothesis", m_clusterHypothesis, m_clusterTree);
  addVariableToTree("ClusterEnergy", m_clusterEnergy, m_clusterTree);
  addVariableToTree("PhotonEnergy", m_photonEnergy, m_clusterTree);
  addVariableToTree("ClusterE1E9", m_clusterE1E9, m_clusterTree);
  addVariableToTree("ClusterErrorTiming", m_clusterErrorTiming, m_clusterTree);
  addVariableToTree("ClusterDetectorRegion", m_clusterDetectorRegion, m_clusterTree);
  addVariableToTree("ClusterMinTrkDistance", m_clusterMinTrkDistance, m_clusterTree);
  addVariableToTree("ClusterDeltaL", m_clusterDeltaL, m_clusterTree);

  addVariableToTree("TrackCluster", m_clusterIsTrack, m_clusterTree);
  addVariableToTree("PhotonCluster", m_clusterIsPhoton, m_clusterTree);
  addVariableToTree("ChargedStableCluster", m_clusterIsChargedStable, m_clusterTree);
}

void ECLTrackClusterMatchingPerformanceModule::writeData()
{
  if (m_tracksTree != nullptr && m_clusterTree != nullptr) {
    TDirectory* oldDir = gDirectory;
    if (m_outputFile)
      m_outputFile->cd();
    m_tracksTree->Write();
    delete m_tracksTree;
    m_clusterTree->Write();
    delete m_clusterTree;
    oldDir->cd();
  }
  if (m_outputFile != nullptr) {
    m_outputFile->Close();
    delete m_outputFile;
  }
}

void ECLTrackClusterMatchingPerformanceModule::setVariablesToDefaultValue()
{
  m_trackProperties = -999;

  m_pValue = -999;

  m_charge = 0;

  m_d0 = -999;

  m_z0 = -999;

  m_mcparticle_cluster_match = 0;

  m_matchedToPhotonHypothesisECLCluster = 0;

  m_matchedToHadronHypothesisECLCluster = 0;

  m_sameclusters = 0;

  m_matchedPhotonHypothesisClusterDetectorRegion = 0;

  m_matchedPhotonHypothesisClusterTheta = -999;

  m_matchedPhotonHypothesisClusterPhi = -999;

  m_matchedPhotonHypothesisClusterDeltaL = -999;

  m_matchedPhotonHypothesisClusterMinTrkDistance = -999;

  m_matchedHadronHypothesisClusterDetectorRegion = 0;

  m_matchedHadronHypothesisClusterTheta = -999;

  m_matchedHadronHypothesisClusterPhi = -999;

  m_matchedHadronHypothesisClusterDeltaL = -999;

  m_matchedHadronHypothesisClusterMinTrkDistance = -999;

  m_mcparticle_cluster_energy = 0.0;

  m_mcparticle_cluster_detectorregion = 0;

  m_mcparticle_cluster_theta = -999;

  m_mcparticle_cluster_phi = -999;
}

void ECLTrackClusterMatchingPerformanceModule::setClusterVariablesToDefaultValue()
{
  m_clusterPhi = -999;

  m_clusterTheta = -999;

  m_clusterIsTrack = 0;

  m_clusterIsPhoton = -999;

  m_clusterIsChargedStable = -999;

  m_clusterHypothesis = 0;

  m_clusterEnergy = -999;

  m_photonEnergy = -999;

  m_clusterE1E9 = -999;

  m_clusterErrorTiming = -999;

  m_clusterDetectorRegion = -999;

  m_clusterMinTrkDistance = -999;

  m_clusterDeltaL = -999;
}

void ECLTrackClusterMatchingPerformanceModule::addVariableToTree(const std::string& varName, double& varReference, TTree* tree)
{
  std::stringstream leaf;
  leaf << varName << "/D";
  tree->Branch(varName.c_str(), &varReference, leaf.str().c_str());
}

void ECLTrackClusterMatchingPerformanceModule::addVariableToTree(const std::string& varName, int& varReference, TTree* tree)
{
  std::stringstream leaf;
  leaf << varName << "/I";
  tree->Branch(varName.c_str(), &varReference, leaf.str().c_str());
}

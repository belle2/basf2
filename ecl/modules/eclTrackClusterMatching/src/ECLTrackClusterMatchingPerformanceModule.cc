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

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/datastore/RelationVector.h>

#include <root/TFile.h>
#include <root/TTree.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ECLTrackClusterMatchingPerformance)

ECLTrackClusterMatchingPerformanceModule::ECLTrackClusterMatchingPerformanceModule() :
  Module(), m_outputFile(NULL), m_dataTree(NULL), m_clusterTree(NULL)
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
  m_recoTracks.isRequired();
  m_trackFitResults.isRequired();
  m_eclClusters.isRequired();
  m_eclShowers.isRequired();

  m_outputFile = new TFile(m_outputFileName.c_str(), "RECREATE");
  TDirectory* oldDir = gDirectory;
  m_outputFile->cd();
  m_dataTree = new TTree("data", "data");
  m_clusterTree = new TTree("cluster", "cluster");
  oldDir->cd();

  setupTree();
}

void ECLTrackClusterMatchingPerformanceModule::event()
{
  StoreObjPtr<EventMetaData> eventMetaData("EventMetaData", DataStore::c_Event);
  m_iEvent = eventMetaData->getEvent();
  m_iRun = eventMetaData->getRun();
  m_iExperiment = eventMetaData->getExperiment();

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
      if (eclCluster.getEnergy() >= 0.5 * relatedMCParticle->getEnergy() && weight >= 0.5 * eclCluster.getEnergy()) {
        if (isChargedStable(*relatedMCParticle)) {
          found_charged_stable = true;
        } else if (relatedMCParticle->getPDG() == 22) {
          found_photon = true;
          m_photonEnergy = relatedMCParticle->getEnergy();
        }
      }
    }
    if (found_mcmatch) {
      if (eclCluster.isTrack()) {
        m_clusterIsTrack = 1;
        const auto& matchingTracks = eclCluster.getRelationsFrom<Track>(m_trackClusterRelationName);
        for (unsigned int index = 0; index < matchingTracks.size(); ++index) {
          const Track* matchingTrack = matchingTracks.object(index);
          if (matchingTrack) {
            m_clusterPt->push_back(matchingTrack->getTrackFitResultWithClosestMass(Const::muon)->getMomentum().Pt());
          }
        }
      }
      m_clusterIsPhoton = int(found_photon);
      m_clusterIsChargedStable = int(found_charged_stable);
      m_clusterPhi = eclCluster.getPhi();
      m_clusterTheta = eclCluster.getTheta();
      m_clusterHypothesis = eclCluster.getHypothesisId();
      m_clusterEnergy = eclCluster.getEnergy();
      m_clusterErrorTiming = eclCluster.getDeltaTime99();
      m_clusterE1E9 = eclCluster.getE1oE9();
      m_clusterDetectorRegion = eclCluster.getDetectorRegion();
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

      const RecoTrack* recoTrack = nullptr;
      double maximumWeight = -2;
      // find highest rated Track
      const auto& relatedRecoTracks = mcParticle.getRelationsWith<RecoTrack>();
      for (unsigned int index = 0; index < relatedRecoTracks.size(); ++index) {
        const RecoTrack* relatedRecoTrack = relatedRecoTracks.object(index);
        const double weight = relatedRecoTracks.weight(index);

        const unsigned int numberOfRelatedTracks = relatedRecoTrack->getRelationsWith<Track>().size();
        B2ASSERT("B2Track <-> RecoTrack is not a 1:1 relation as expected!", numberOfRelatedTracks <= 1);
        // use only the fitted reco tracks
        if (numberOfRelatedTracks == 1) {
          if (weight > maximumWeight) {
            maximumWeight = weight;
            recoTrack = relatedRecoTrack;
          }
        }
      }

      if (recoTrack) {
        const Track* b2Track = recoTrack->getRelated<Track>();
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

        double shower_energy = 0., highest_track_related_shower_energy = 0.;
        const ECLCluster* eclCluster_WithHighestEnergy_track_related = nullptr;
        for (const auto& eclCluster : b2Track->getRelationsTo<ECLCluster>(m_trackClusterRelationName)) {
          if (eclCluster.getHypothesisId() != 5) continue;
          if (!(eclCluster.isTrack())) continue;
          const ECLShower* eclShower = eclCluster.getRelatedTo<ECLShower>();
          shower_energy = eclShower->getEnergy();
          if (shower_energy > highest_track_related_shower_energy) {
            highest_track_related_shower_energy = shower_energy;
            eclCluster_WithHighestEnergy_track_related = &eclCluster;
          }
        }
        if (eclCluster_WithHighestEnergy_track_related != nullptr) {
          m_matchedToECLCluster = 1;
          m_hypothesisOfMatchedECLCluster = eclCluster_WithHighestEnergy_track_related->getHypothesisId();
          m_detectorRegion = eclCluster_WithHighestEnergy_track_related->getDetectorRegion();
          m_matchedClusterTheta = eclCluster_WithHighestEnergy_track_related->getTheta();
          m_matchedClusterPhi = eclCluster_WithHighestEnergy_track_related->getPhi();
        }
        // find ECLCluster in which the particle has deposited the majority of its energy
        maximumWeight = -2.;
        const ECLCluster* eclCluster_matchedBestToMCParticle = nullptr;
        const auto& relatedECLClusters = mcParticle.getRelationsFrom<ECLCluster>();
        for (unsigned int index = 0; index < relatedECLClusters.size(); ++index) {
          const ECLCluster* relatedECLCluster = relatedECLClusters.object(index);
          if (relatedECLCluster->getHypothesisId() != 5) continue;
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
          if (eclCluster_WithHighestEnergy_track_related == eclCluster_matchedBestToMCParticle) {
            m_sameclusters = 1;
          }
        }
      }

      m_dataTree->Fill(); // write data to tree
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
  if (m_dataTree == NULL || m_clusterTree == NULL) {
    B2FATAL("Data tree or event tree was not created.");
  }
  addVariableToTree("expNo", m_iExperiment, m_dataTree);
  addVariableToTree("runNo", m_iRun, m_dataTree);
  addVariableToTree("evtNo", m_iEvent, m_dataTree);

  addVariableToTree("pdgCode", m_trackProperties.pdg_gen, m_dataTree);

  addVariableToTree("cosTheta", m_trackProperties.cosTheta, m_dataTree);
  addVariableToTree("cosTheta_gen",  m_trackProperties.cosTheta_gen, m_dataTree);

  addVariableToTree("phi", m_trackProperties.phi, m_dataTree);
  addVariableToTree("phi_gen", m_trackProperties.phi_gen, m_dataTree);

  addVariableToTree("px", m_trackProperties.px, m_dataTree);
  addVariableToTree("px_gen", m_trackProperties.px_gen, m_dataTree);

  addVariableToTree("py", m_trackProperties.py, m_dataTree);
  addVariableToTree("py_gen", m_trackProperties.py_gen, m_dataTree);

  addVariableToTree("pz", m_trackProperties.pz, m_dataTree);
  addVariableToTree("pz_gen", m_trackProperties.pz_gen, m_dataTree);

  addVariableToTree("x", m_trackProperties.x, m_dataTree);
  addVariableToTree("x_gen", m_trackProperties.x_gen, m_dataTree);

  addVariableToTree("y", m_trackProperties.y, m_dataTree);
  addVariableToTree("y_gen", m_trackProperties.y_gen, m_dataTree);

  addVariableToTree("z", m_trackProperties.z, m_dataTree);
  addVariableToTree("z_gen", m_trackProperties.z_gen, m_dataTree);

  addVariableToTree("pt", m_trackProperties.pt, m_dataTree);
  addVariableToTree("pt_gen", m_trackProperties.pt_gen, m_dataTree);

  addVariableToTree("ptot", m_trackProperties.ptot, m_dataTree);
  addVariableToTree("ptot_gen", m_trackProperties.ptot_gen, m_dataTree);

  addVariableToTree("pValue", m_pValue, m_dataTree);

  addVariableToTree("charge", m_charge, m_dataTree);

  addVariableToTree("d0", m_d0, m_dataTree);
  addVariableToTree("z0", m_z0, m_dataTree);

  addVariableToTree("nPXDhits", m_trackProperties.nPXDhits, m_dataTree);
  addVariableToTree("nSVDhits", m_trackProperties.nSVDhits, m_dataTree);
  addVariableToTree("nCDChits", m_trackProperties.nCDChits, m_dataTree);

  addVariableToTree("ECLMatch", m_matchedToECLCluster, m_dataTree);
  addVariableToTree("ShowerMatch", m_sameclusters, m_dataTree);
  addVariableToTree("MCParticleClusterMatch", m_mcparticle_cluster_match, m_dataTree);

  addVariableToTree("DetectorRegion", m_detectorRegion, m_dataTree);
  addVariableToTree("HypothesisID", m_hypothesisOfMatchedECLCluster, m_dataTree);
  addVariableToTree("ClusterTheta", m_matchedClusterTheta, m_dataTree);
  addVariableToTree("ClusterPhi", m_matchedClusterPhi, m_dataTree);

  addVariableToTree("MCClusterEnergy", m_mcparticle_cluster_energy, m_dataTree);
  addVariableToTree("MCClusterDetectorRegion", m_mcparticle_cluster_detectorregion, m_dataTree);
  addVariableToTree("MCClusterTheta", m_mcparticle_cluster_theta, m_dataTree);
  addVariableToTree("MCClusterPhi", m_mcparticle_cluster_phi, m_dataTree);


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
  m_clusterTree->Branch("ClusterPT", "std::vector<double>", &m_clusterPt);

  addVariableToTree("TrackCluster", m_clusterIsTrack, m_clusterTree);
  addVariableToTree("PhotonCluster", m_clusterIsPhoton, m_clusterTree);
  addVariableToTree("ChargedStableCluster", m_clusterIsChargedStable, m_clusterTree);
}

void ECLTrackClusterMatchingPerformanceModule::writeData()
{
  if (m_dataTree != NULL && m_clusterTree != NULL) {
    TDirectory* oldDir = gDirectory;
    if (m_outputFile)
      m_outputFile->cd();
    m_dataTree->Write();
    delete m_dataTree;
    m_clusterTree->Write();
    delete m_clusterTree;
    oldDir->cd();
  }
  if (m_outputFile != NULL) {
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

  m_matchedToECLCluster = 0;

  m_sameclusters = 0;

  m_detectorRegion = 0;

  m_hypothesisOfMatchedECLCluster = 0;

  m_matchedClusterTheta = -999;

  m_matchedClusterPhi = -999;

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

  m_clusterPt->clear();
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

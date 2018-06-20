/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Frank Meier                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <ecl/modules/eclTrackingPerformance/ECLTrackingPerformanceModule.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/datastore/RelationVector.h>

#include <pxd/reconstruction/PXDRecoHit.h>
#include <svd/reconstruction/SVDRecoHit.h>
#include <svd/reconstruction/SVDRecoHit2D.h>
#include <cdc/dataobjects/CDCRecoHit.h>

#include <root/TFile.h>
#include <root/TTree.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ECLTrackingPerformance)

ECLTrackingPerformanceModule::ECLTrackingPerformanceModule() :
  Module(), m_outputFile(NULL), m_dataTree(NULL), m_eventTree(NULL)
{
  setDescription("Module to test the track cluster matching efficiency. Writes information about the tracks and MCParticles in a ROOT file.");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("outputFileName", m_outputFileName, "Name of output root file.",
           std::string("ECLTrackingPerformanceOutput.root"));
  addParam("recoTracksStoreArrayName", m_recoTracksStoreArrayName, "Name of the RecoTracks StoreArray.",
           std::string(""));
}

void ECLTrackingPerformanceModule::initialize()
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
  m_eventTree = new TTree("fake", "fake");
  oldDir->cd();

  setupTree();
}

void ECLTrackingPerformanceModule::event()
{
  StoreObjPtr<EventMetaData> eventMetaData("EventMetaData", DataStore::c_Event);
  m_iEvent = eventMetaData->getEvent();
  m_iRun = eventMetaData->getRun();
  m_iExperiment = eventMetaData->getExperiment();

  B2DEBUG(99, "Processes experiment " << m_iExperiment << " run " << m_iRun << " event " << m_iEvent);

  m_photonclusters = 0;
  m_fakeclusters = 0;

  for (const ECLCluster& eclCluster : m_eclClusters) {
    if (eclCluster.getHypothesisId() != 5) continue;
    double maximumWeight = -2.;
    const MCParticle* mcParticle_with_highest_weight = nullptr;
    const auto& relatedMCParticles = eclCluster.getRelationsWith<MCParticle>();
    for (unsigned int index = 0; index < relatedMCParticles.size(); ++index) {
      const MCParticle* relatedMCParticle = relatedMCParticles.object(index);
      const double weight = relatedMCParticles.weight(index);
      if (weight > maximumWeight) {
        mcParticle_with_highest_weight = relatedMCParticle;
      }
    }
    if (mcParticle_with_highest_weight != nullptr && mcParticle_with_highest_weight->getPDG() == 22) {
      m_photonclusters++;
      if (eclCluster.isTrack()) {
        m_fakeclusters++;
      }
    }
  }
  m_eventTree->Fill();

  for (const MCParticle& mcParticle : m_mcParticles) {
    // check status of mcParticle
    if (isPrimaryMcParticle(mcParticle) && isChargedStable(mcParticle) && mcParticle.hasStatus(MCParticle::c_StableInGenerator)) {
      setVariablesToDefaultValue();

      int pdgCode = mcParticle.getPDG();
      B2DEBUG(99, "Primary MCParticle has PDG code " << pdgCode);
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
      const auto& relatedRecoTracks = mcParticle.getRelationsWith<RecoTrack>(m_recoTracksStoreArrayName);
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
        const TrackFitResult* fitResult = b2Track->getTrackFitResultWithClosestMass(Const::muon);
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
        m_trackProperties.nPXDhits = recoTrack->getNumberOfPXDHits();
        m_trackProperties.nSVDhits = recoTrack->getNumberOfSVDHits();
        m_trackProperties.nCDChits = recoTrack->getNumberOfCDCHits();

        double shower_energy = 0., highest_track_related_shower_energy = 0.;
        const ECLCluster* eclCluster_WithHighestEnergy_track_related = nullptr;
        for (auto& eclCluster : b2Track->getRelationsTo<ECLCluster>()) {
          if (eclCluster.getHypothesisId() != 5) continue;
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
        }
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
          if (eclCluster_WithHighestEnergy_track_related == eclCluster_matchedBestToMCParticle) {
            m_sameclusters = 1;
          }
        }
      }

      m_dataTree->Fill(); // write data to tree
    }
  }
}


void ECLTrackingPerformanceModule::terminate()
{
  writeData();
}

bool ECLTrackingPerformanceModule::isPrimaryMcParticle(const MCParticle& mcParticle)
{
  return mcParticle.hasStatus(MCParticle::c_PrimaryParticle);
}

bool ECLTrackingPerformanceModule::isChargedStable(const MCParticle& mcParticle)
{
  return Const::chargedStableSet.find(abs(mcParticle.getPDG()))
         != Const::invalidParticle;
}

void ECLTrackingPerformanceModule::setupTree()
{
  if (m_dataTree == NULL || m_eventTree == NULL) {
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
  addVariableToTree("HypothesisID", m_hypothesisOfMatchedECLCluster, m_dataTree);
  addVariableToTree("ShowerMatch", m_sameclusters, m_dataTree);
  addVariableToTree("MCParticleClusterMatch", m_mcparticle_cluster_match, m_dataTree);
  addVariableToTree("ClusterEnergy", m_mcparticle_cluster_energy, m_dataTree);


  addVariableToTree("expNo", m_iExperiment, m_eventTree);
  addVariableToTree("runNo", m_iRun, m_eventTree);
  addVariableToTree("evtNo", m_iEvent, m_eventTree);

  addVariableToTree("PhotonCluster", m_photonclusters, m_eventTree);
  addVariableToTree("FakeCluster", m_fakeclusters, m_eventTree);
}

void ECLTrackingPerformanceModule::writeData()
{
  if (m_dataTree != NULL && m_eventTree != NULL) {
    TDirectory* oldDir = gDirectory;
    if (m_outputFile)
      m_outputFile->cd();
    m_dataTree->Write();
    m_eventTree->Write();
    oldDir->cd();
  }
  if (m_outputFile != NULL) {
    m_outputFile->Close();
  }
}

void ECLTrackingPerformanceModule::setVariablesToDefaultValue()
{
  m_trackProperties = -999;

  m_pValue = -999;

  m_charge = 0;

  m_d0 = -999;

  m_z0 = -999;

  m_mcparticle_cluster_match = 0;

  m_matchedToECLCluster = 0;

  m_hypothesisOfMatchedECLCluster = 0;

  m_sameclusters = 0;

  m_mcparticle_cluster_energy = 0.0;
}

void ECLTrackingPerformanceModule::addVariableToTree(const std::string& varName, double& varReference, TTree* tree)
{
  std::stringstream leaf;
  leaf << varName << "/D";
  tree->Branch(varName.c_str(), &varReference, leaf.str().c_str());
}

void ECLTrackingPerformanceModule::addVariableToTree(const std::string& varName, int& varReference, TTree* tree)
{
  std::stringstream leaf;
  leaf << varName << "/I";
  tree->Branch(varName.c_str(), &varReference, leaf.str().c_str());
}

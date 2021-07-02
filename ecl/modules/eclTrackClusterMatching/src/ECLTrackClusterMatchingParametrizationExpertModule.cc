/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <ecl/modules/eclTrackClusterMatching/ECLTrackClusterMatchingParametrizationExpertModule.h>
#include <framework/datastore/RelationVector.h>
#include <framework/gearbox/Const.h>
#include <vector>
#include <cmath>
#include "TFile.h"
#include "TTree.h"

using namespace std;
using namespace Belle2;

REG_MODULE(ECLTrackClusterMatchingParametrizationExpert)

ECLTrackClusterMatchingParametrizationExpertModule::ECLTrackClusterMatchingParametrizationExpertModule() : Module(),
  m_writeToRoot(1),
  m_iExperiment(0),
  m_iRun(0),
  m_iEvent(0),
  m_trackNo(0),
  m_trackMomentum(0),
  m_pT(0),
  m_trackTheta(0),
  m_deltaPhi(0),
  m_phiCluster(0),
  m_phiHit(0),
  m_errorPhi(0),
  m_deltaTheta(0),
  m_thetaCluster(0),
  m_thetaHit(0),
  m_errorTheta(0),
  m_hitstatus(0),
  m_true_track_pdg(0),
  m_true_match(0)
{
  setDescription("Store ExtHit related infos for track cluster matching");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("writeToRoot", m_writeToRoot,
           "set true if you want to save the information in a root file named by parameter 'rootFileName'", bool(false));
  addParam("useArray", m_useArray,
           "set true if you want to save the information event-wise using an array structure", bool(false));
  addParam("rootFileName", m_rootFileName,
           "fileName used for root file where info are saved. Will be ignored if parameter 'writeToRoot' is false (standard)",
           string("eclTrackClusterMatchingAnalysis.root"));
}

ECLTrackClusterMatchingParametrizationExpertModule::~ECLTrackClusterMatchingParametrizationExpertModule()
{
}

void ECLTrackClusterMatchingParametrizationExpertModule::initialize()
{
  // Check dependencies
  m_tracks.isRequired();
  m_eclClusters.isRequired();
  m_tracks.registerRelationTo(m_eclClusters);
  m_extHits.isRequired();
  m_trackFitResults.isRequired();
  m_mcParticles.isRequired();

  m_eventMetaData.isOptional();

  m_rootFilePtr = new TFile(m_rootFileName.c_str(), "RECREATE");
  m_tree     = new TTree("tree", "ECL Track Cluster Matching Analysis tree");

  m_tree->Branch("expNo", &m_iExperiment, "expNo/I");
  m_tree->Branch("runNo", &m_iRun, "runNo/I");
  m_tree->Branch("evtNo", &m_iEvent, "evtNo/I");

  if (m_useArray) {
    m_tree->Branch("trackNo", "std::vector<int>", &m_trackNo_array);
    m_tree->Branch("trackMomentum", "std::vector<float>", &m_trackMomentum_array);
    m_tree->Branch("pT", "std::vector<float>", &m_pT_array);
    m_tree->Branch("trackTheta", "std::vector<float>", &m_trackTheta_array);
    m_tree->Branch("deltaPhi", "std::vector<float>",  &m_deltaPhi_array);
    m_tree->Branch("phiCluster", "std::vector<float>",  &m_phiCluster_array);
    m_tree->Branch("phiHit", "std::vector<float>",  &m_phiHit_array);
    m_tree->Branch("errorPhi", "std::vector<float>",  &m_errorPhi_array);
    m_tree->Branch("deltaTheta", "std::vector<float>",  &m_deltaTheta_array);
    m_tree->Branch("thetaCluster", "std::vector<float>",  &m_thetaCluster_array);
    m_tree->Branch("thetaHit", "std::vector<float>",  &m_thetaHit_array);
    m_tree->Branch("errorTheta", "std::vector<float>",  &m_errorTheta_array);
    m_tree->Branch("hitStatus", "std::vector<int>",  &m_hitstatus_array);
    m_tree->Branch("trueTrackPDG", "std::vector<int>",  &m_true_track_pdg_array);
    m_tree->Branch("trueMatch", "std::vector<int>",  &m_true_match_array);
  } else {
    m_tree->Branch("trackNo", &m_trackNo, "trackNo/I");
    m_tree->Branch("trackMomentum", &m_trackMomentum, "trackMomentum/F");
    m_tree->Branch("pT", &m_pT, "pT/F");
    m_tree->Branch("trackTheta", &m_trackTheta, "trackTheta/F");
    m_tree->Branch("deltaPhi", &m_deltaPhi, "deltaPhi/F");
    m_tree->Branch("phiCluster", &m_phiCluster, "phiCluster/F");
    m_tree->Branch("phiHit", &m_phiHit, "phiHit/F");
    m_tree->Branch("errorPhi", &m_errorPhi, "errorPhi/F");
    m_tree->Branch("deltaTheta", &m_deltaTheta, "deltaTheta/F");
    m_tree->Branch("thetaCluster", &m_thetaCluster, "thetaCluster/F");
    m_tree->Branch("thetaHit", &m_thetaHit, "thetaHit/F");
    m_tree->Branch("errorTheta", &m_errorTheta, "errorTheta/F");
    m_tree->Branch("hitStatus", &m_hitstatus, "hitStatus/I");
    m_tree->Branch("trueTrackPDG", &m_true_track_pdg, "trueTrackPDG/I");
    m_tree->Branch("trueMatch", &m_true_match, "trueMatch/I");
  }
}

void ECLTrackClusterMatchingParametrizationExpertModule::event()
{
  if (m_useArray) {
    m_deltaPhi_array->clear();
    m_phiCluster_array->clear();
    m_phiHit_array->clear();
    m_errorPhi_array->clear();
    m_deltaTheta_array->clear();
    m_thetaCluster_array->clear();
    m_thetaHit_array->clear();
    m_errorTheta_array->clear();
    m_trackNo_array->clear();
    m_trackMomentum_array->clear();
    m_pT_array->clear();
    m_trackTheta_array->clear();
    m_hitstatus_array->clear();
    m_true_track_pdg_array->clear();
    m_true_match_array->clear();
  }

  if (m_eventMetaData) {
    m_iExperiment = m_eventMetaData->getExperiment();
    m_iRun = m_eventMetaData->getRun();
    m_iEvent = m_eventMetaData->getEvent();
  } else {
    m_iExperiment = -1;
    m_iRun = -1;
    m_iEvent = -1;
  }

  int i = 0;

  for (const Track& track : m_tracks) {

    const MCParticle* relatedMCParticle = track.getRelatedTo<MCParticle>();
    if (!relatedMCParticle) continue;
    int pdgCode = relatedMCParticle->getPDG();
    Const::ChargedStable hypothesis = Const::pion;
    if (Const::chargedStableSet.find(abs(pdgCode)) != Const::invalidParticle) {
      hypothesis = Const::ChargedStable(abs(pdgCode));
    }
    const TrackFitResult* fitResult = track.getTrackFitResultWithClosestMass(hypothesis);

    double momentum = fitResult->getMomentum().Mag();
    double pt = fitResult->getTransverseMomentum();
    double theta = TMath::ACos(fitResult->getMomentum().CosTheta());
    if (m_useArray) {
      m_true_track_pdg_array->push_back(pdgCode);
      m_trackNo_array->push_back(i);
      m_trackMomentum_array->push_back(momentum);
      m_pT_array->push_back(pt);
      m_trackTheta_array->push_back(theta);
    } else {
      m_true_track_pdg = pdgCode;
      m_trackNo = i;
      m_trackMomentum = momentum;
      m_pT = pt;
      m_trackTheta = theta;
    }
    i++;
    // Find extrapolated track hits in the ECL, considering only hit points
    // that either are on the sphere, closest to, or on radial direction of an
    // ECLCluster.
    for (const auto& extHit : track.getRelationsTo<ExtHit>()) {
      if (!isECLHit(extHit)) continue;
      ECLCluster* eclCluster = extHit.getRelatedFrom<ECLCluster>();
      if (eclCluster != nullptr) {
        // only use c_nPhotons clusters
        if (!eclCluster->hasHypothesis(ECLCluster::EHypothesisBit::c_nPhotons)) continue;
        double errorPhi = extHit.getErrorPhi();
        double errorTheta = extHit.getErrorTheta();
        double phiHit = extHit.getPosition().Phi();
        double phiCluster = eclCluster->getPhi();
        double deltaPhi = phiHit - phiCluster;
        if (deltaPhi > M_PI) {
          deltaPhi = deltaPhi - 2 * M_PI;
        } else if (deltaPhi < -M_PI) {
          deltaPhi = deltaPhi + 2 * M_PI;
        }
        double thetaHit = extHit.getPosition().Theta();
        double thetaCluster = eclCluster->getTheta();
        double deltaTheta = thetaHit - thetaCluster;
        ExtHitStatus hitStatus = extHit.getStatus();
        const auto& relatedMCParticles = eclCluster->getRelationsTo<MCParticle>();
        bool found_match = false;
        for (unsigned int index = 0; index < relatedMCParticles.size() && !found_match; ++index) {
          const MCParticle* clusterRelatedMCParticle = relatedMCParticles.object(index);
          if (clusterRelatedMCParticle == relatedMCParticle) {
            found_match = true;
          }
        }
        if (m_useArray) {
          m_errorPhi_array->push_back(errorPhi);
          m_errorTheta_array->push_back(errorTheta);
          m_deltaPhi_array->push_back(deltaPhi);
          m_phiCluster_array->push_back(phiCluster);
          m_phiHit_array->push_back(phiHit);
          m_deltaTheta_array->push_back(deltaTheta);
          m_thetaCluster_array->push_back(thetaCluster);
          m_thetaHit_array->push_back(thetaHit);
          m_hitstatus_array->push_back(hitStatus);
          m_true_match_array->push_back(int(found_match));
        } else {
          m_errorPhi = errorPhi;
          m_errorTheta = errorTheta;
          m_deltaPhi = deltaPhi;
          m_phiCluster = phiCluster;
          m_phiHit = phiHit;
          m_deltaTheta = deltaTheta;
          m_thetaCluster = thetaCluster;
          m_thetaHit = thetaHit;
          m_hitstatus = hitStatus;
          m_true_match = int(found_match);
          m_tree->Fill();
        }
      }
    } // end loop on ExtHits related to Track
  } // end loop on Tracks
  if (m_useArray) m_tree->Fill();
}

void ECLTrackClusterMatchingParametrizationExpertModule::terminate()
{
  if (m_rootFilePtr != nullptr) {
    m_rootFilePtr->cd();
    m_tree->Write();
    delete m_tree;
    m_rootFilePtr->Close();
    delete m_rootFilePtr;
  }
}

bool ECLTrackClusterMatchingParametrizationExpertModule::isECLHit(const ExtHit& extHit) const
{
  if ((extHit.getDetectorID() != Const::EDetector::ECL)) return false;
  ExtHitStatus extHitStatus = extHit.getStatus();
  if (extHitStatus == EXT_ECLCROSS || extHitStatus == EXT_ECLDL || extHitStatus == EXT_ECLNEAR) return true;
  else return false;
}


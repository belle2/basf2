/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 * Contributors: Frank Meier                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <ecl/modules/eclTrackClusterMatching/ECLTrackClusterMatchingParametrizationExpertModule.h>
#include <framework/datastore/RelationVector.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <set>
#include <vector>
#include <cmath>

using namespace std;
using namespace Belle2;

REG_MODULE(ECLTrackClusterMatchingParametrizationExpert)

ECLTrackClusterMatchingParametrizationExpertModule::ECLTrackClusterMatchingParametrizationExpertModule() : Module(),
  m_rootFilePtr(0),
  m_writeToRoot(1),
  m_tree(0),
  m_iExperiment(0),
  m_iRun(0),
  m_iEvent(0),
  m_trackNo(0),
  m_trackMomentum(0),
  m_pT(0),
  m_deltaPhi(0),
  m_phiCluster(0),
  m_phiHit(0),
  m_errorPhi(0),
  m_deltaTheta(0),
  m_thetaCluster(0),
  m_thetaHit(0),
  m_errorTheta(0),
  m_hitstatus(0),
  m_true_track_pdg(0)
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

  m_eventMetaData.isOptional();
  m_mcParticles.isOptional();

  m_rootFilePtr = new TFile(m_rootFileName.c_str(), "RECREATE");
  m_tree     = new TTree("m_tree", "ECL Track Cluster Matching Analysis tree");

  m_tree->Branch("expNo", &m_iExperiment, "expNo/I");
  m_tree->Branch("runNo", &m_iRun, "runNo/I");
  m_tree->Branch("evtNo", &m_iEvent, "evtNo/I");

  if (m_useArray) {
    m_tree->Branch("trackNo", "std::vector<int>", &m_trackNo_array);
    m_tree->Branch("trackMomentum", "std::vector<double>", &m_trackMomentum_array);
    m_tree->Branch("deltaPhi", "std::vector<double>",  &m_deltaPhi_array);
    m_tree->Branch("phiCluster", "std::vector<double>",  &m_phiCluster_array);
    m_tree->Branch("phiHit", "std::vector<double>",  &m_phiHit_array);
    m_tree->Branch("errorPhi", "std::vector<double>",  &m_errorPhi_array);
    m_tree->Branch("deltaTheta", "std::vector<double>",  &m_deltaTheta_array);
    m_tree->Branch("thetaCluster", "std::vector<double>",  &m_thetaCluster_array);
    m_tree->Branch("thetaHit", "std::vector<double>",  &m_thetaHit_array);
    m_tree->Branch("errorTheta", "std::vector<double>",  &m_errorTheta_array);
    m_tree->Branch("hitStatus", "std::vector<int>",  &m_hitstatus_array);
    m_tree->Branch("trueTrackPDG", "std::vector<int>",  &m_true_track_pdg_array);
  } else {
    m_tree->Branch("trackNo", &m_trackNo, "trackNo/I");
    m_tree->Branch("trackMomentum", &m_trackMomentum, "trackMomentum/D");
    m_tree->Branch("pT", &m_pT, "pT/D");
    m_tree->Branch("deltaPhi", &m_deltaPhi, "deltaPhi/D");
    m_tree->Branch("phiCluster", &m_phiCluster, "phiCluster/D");
    m_tree->Branch("phiHit", &m_phiHit, "phiHit/D");
    m_tree->Branch("errorPhi", &m_errorPhi, "errorPhi/D");
    m_tree->Branch("deltaTheta", &m_deltaTheta, "deltaTheta/D");
    m_tree->Branch("thetaCluster", &m_thetaCluster, "thetaCluster/D");
    m_tree->Branch("thetaHit", &m_thetaHit, "thetaHit/D");
    m_tree->Branch("errorTheta", &m_errorTheta, "errorTheta/D");
    m_tree->Branch("hitStatus", &m_hitstatus, "hitStatus/I");
    m_tree->Branch("trueTrackPDG", &m_true_track_pdg, "trueTrackPDG/I");
  }
}

void ECLTrackClusterMatchingParametrizationExpertModule::beginRun()
{
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
    m_hitstatus_array->clear();
    m_true_track_pdg_array->clear();
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

    const TrackFitResult* fitResult = track.getTrackFitResultWithClosestMass(Const::muon);
    double momentum = fitResult->getMomentum().Mag();
    double pt = fitResult->getTransverseMomentum();
    const MCParticle* relatedMCParticle = track.getRelatedTo<MCParticle>();
    if (relatedMCParticle) {
      if (m_useArray) m_true_track_pdg_array->push_back(relatedMCParticle->getPDG());
      else m_true_track_pdg = relatedMCParticle->getPDG();
    }
    if (m_useArray) {
      m_trackNo_array->push_back(i);
      m_trackMomentum_array->push_back(momentum);
    } else {
      m_trackNo = i;
      m_trackMomentum = momentum;
      m_pT = pt;
    }
    i++;
    // Find extrapolated track hits in the ECL, considering only hit points
    // that either are on the sphere, closest to, or on radial direction of an
    // ECLCluster.
    for (const auto& extHit : track.getRelationsTo<ExtHit>()) {
      if (!isECLHit(extHit)) continue;
      ECLCluster* eclCluster = extHit.getRelatedFrom<ECLCluster>();
      if (eclCluster != nullptr) {
        if (eclCluster->getHypothesisId() != 5) continue;
        double errorPhi = extHit.getErrorPhi();
        // if (errorPhi > 2 * M_PI) continue;
        double errorTheta = extHit.getErrorTheta();
        // if (errorTheta > M_PI) continue;
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
        // if (phiHit > 0 && phiCluster < 0) deltaTheta = thetaHit + thetaCluster;
        // else if (phiHit < 0 && phiCluster > 0) deltaTheta = -thetaHit - thetaCluster;
        // else if (phiHit < 0 && phiCluster < 0) deltaTheta = -thetaHit + thetaCluster;
        // if (deltaTheta > M_PI) {
        //   deltaTheta = deltaTheta - 2 * M_PI;
        // } else if (deltaTheta < -M_PI) {
        //   deltaTheta = deltaTheta + 2 * M_PI;
        // }
        ExtHitStatus hitStatus = extHit.getStatus();
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
          m_tree->Fill();
        }
      }
    } // end loop on ExtHits related to Track
  } // end loop on Tracks
  if (m_useArray) m_tree->Fill();
}

void ECLTrackClusterMatchingParametrizationExpertModule::endRun()
{
}

void ECLTrackClusterMatchingParametrizationExpertModule::terminate()
{
  if (m_rootFilePtr != NULL) {
    m_rootFilePtr->cd();
    m_tree->Write();
    m_rootFilePtr->Close();
  }
}

bool ECLTrackClusterMatchingParametrizationExpertModule::isECLHit(const ExtHit& extHit) const
{
  if ((extHit.getDetectorID() != Const::EDetector::ECL)) return false;
  ExtHitStatus extHitStatus = extHit.getStatus();
  if (extHitStatus == EXT_ECLCROSS || extHitStatus == EXT_ECLDL || extHitStatus == EXT_ECLNEAR) return true;
  else return false;
}


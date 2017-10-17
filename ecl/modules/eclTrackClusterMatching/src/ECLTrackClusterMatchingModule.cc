/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 * Contributors: Frank Meier                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <ecl/modules/eclTrackClusterMatching/ECLTrackClusterMatchingModule.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/RelationVector.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <set>
#include <vector>
#include <cmath>

using namespace std;
using namespace Belle2;

REG_MODULE(ECLTrackClusterMatching)

ECLTrackClusterMatchingModule::ECLTrackClusterMatchingModule()
  : Module(),
    m_rootFilePtr(0),
    m_writeToRoot(1),
    m_tree(0),
    m_iExperiment(0),
    m_iRun(0),
    m_iEvent(0),
    m_trackNo(0),
    m_trackMomentum(0),
    m_deltaPhi(0),
    m_phiCluster(0),
    m_errorPhi_ECLNEAR(0),
    m_errorPhi_ECLCROSS(0),
    m_errorPhi_ECLDL(0),
    m_deltaTheta(0),
    m_thetaCluster(0),
    m_errorTheta_ECLNEAR(0),
    m_errorTheta_ECLCROSS(0),
    m_errorTheta_ECLDL(0),
    m_quality(0),
    m_quality_best(0),
    m_hitstatus_best(0)
{
  setDescription("Match Tracks to ECLCluster");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("writeToRoot", m_writeToRoot,
           "set true if you want to save the information in a root file named by parameter 'rootFileName'", bool(false));
  addParam("rootFileName", m_rootFileName,
           "fileName used for root file where info are saved. Will be ignored if parameter 'writeToRoot' is false (standard)",
           string("eclTrackClusterMatchingAnalysis.root"));
}

ECLTrackClusterMatchingModule::~ECLTrackClusterMatchingModule()
{
}

void ECLTrackClusterMatchingModule::initialize()
{
  B2INFO("[ECLTrackClusterMatching Module]: Starting initialization of ECLTrackClusterMatching Module.");

  StoreArray<Track> tracks;
  StoreArray<ECLCluster> eclClusters;
  tracks.registerRelationTo(eclClusters);
  tracks.registerRelationTo(tracks);

  if (m_writeToRoot == true) {
    m_rootFilePtr = new TFile(m_rootFileName.c_str(), "RECREATE");
  } else
    m_rootFilePtr = NULL;

  // initialize tree
  m_tree     = new TTree("m_tree", "ECL Track Cluster Matching Analysis tree");

  m_tree->Branch("expNo", &m_iExperiment, "expNo/I");
  m_tree->Branch("runNo", &m_iRun, "runNo/I");
  m_tree->Branch("evtNo", &m_iEvent, "evtNo/I");
  m_tree->Branch("trackNo", "std::vector<int>", &m_trackNo);

  m_tree->Branch("trackMomentum", "std::vector<double>", &m_trackMomentum);
  m_tree->Branch("deltaPhi", "std::vector<double>",  &m_deltaPhi);
  m_tree->Branch("phiCluster", "std::vector<double>",  &m_phiCluster);
  m_tree->Branch("errorPhi_ECLNEAR", "std::vector<double>",  &m_errorPhi_ECLNEAR);
  m_tree->Branch("errorPhi_ECLCROSS", "std::vector<double>",  &m_errorPhi_ECLCROSS);
  m_tree->Branch("errorPhi_ECLDL", "std::vector<double>",  &m_errorPhi_ECLDL);
  m_tree->Branch("deltaTheta", "std::vector<double>",  &m_deltaTheta);
  m_tree->Branch("thetaCluster", "std::vector<double>",  &m_thetaCluster);
  m_tree->Branch("errorTheta_ECLNEAR", "std::vector<double>",  &m_errorTheta_ECLNEAR);
  m_tree->Branch("errorTheta_ECLCROSS", "std::vector<double>",  &m_errorTheta_ECLCROSS);
  m_tree->Branch("errorTheta_ECLDL", "std::vector<double>",  &m_errorTheta_ECLDL);
  m_tree->Branch("quality", "std::vector<double>",  &m_quality);
  m_tree->Branch("quality_best", "std::vector<double>",  &m_quality_best);
  m_tree->Branch("hitStatus_best", "std::vector<int>",  &m_hitstatus_best);

  B2INFO("[ECLTrackClusterMatchingModule]: Initialization of ECLTrackClusterMatching Module completed.");
}

void ECLTrackClusterMatchingModule::beginRun()
{
}

void ECLTrackClusterMatchingModule::event()
{
  m_deltaPhi->clear();
  m_phiCluster->clear();
  m_errorPhi_ECLNEAR->clear();
  m_errorPhi_ECLCROSS->clear();
  m_errorPhi_ECLDL->clear();
  m_deltaTheta->clear();
  m_thetaCluster->clear();
  m_errorTheta_ECLNEAR->clear();
  m_errorTheta_ECLCROSS->clear();
  m_errorTheta_ECLDL->clear();
  m_quality->clear();
  m_quality_best->clear();
  m_trackNo->clear();
  m_trackMomentum->clear();
  m_hitstatus_best->clear();

  StoreObjPtr<EventMetaData> eventmetadata;
  if (eventmetadata) {
    m_iExperiment = eventmetadata->getExperiment();
    m_iRun = eventmetadata->getRun();
    m_iEvent = eventmetadata->getEvent();
  } else {
    m_iExperiment = -1;
    m_iRun = -1;
    m_iEvent = -1;
  }

  StoreArray<Track> tracks;
  StoreArray<ECLCluster> eclClusters;

  int i = 0;

  for (const Track& track : tracks) {

    ECLCluster* cluster_best = nullptr;
    double quality_tmp = 1e6;
    ExtHitStatus hitStatus;
    i++;
    // Find extrapolated track hits in the ECL, considering only hit points
    // that either are on the sphere, closest to or on radial direction of an
    // ECLCluster.
    for (const auto& extHit : track.getRelationsTo<ExtHit>()) {
      if (!isECLHit(extHit)) continue;
      ECLCluster* eclCluster = extHit.getRelatedFrom<ECLCluster>();
      if (eclCluster != nullptr) {
        if (eclCluster->getHypothesisId() != 5) continue;
        if (extHit.getStatus() == EXT_ECLNEAR) {
          m_errorPhi_ECLNEAR->push_back(extHit.getErrorPhi());
          m_errorTheta_ECLNEAR->push_back(extHit.getErrorTheta());
          m_errorPhi_ECLCROSS->push_back(-1);
          m_errorTheta_ECLCROSS->push_back(-1);
          m_errorPhi_ECLDL->push_back(-1);
          m_errorTheta_ECLDL->push_back(-1);
        } else if (extHit.getStatus() == EXT_ECLCROSS) {
          m_errorPhi_ECLNEAR->push_back(-1);
          m_errorTheta_ECLNEAR->push_back(-1);
          m_errorPhi_ECLCROSS->push_back(extHit.getErrorPhi());
          m_errorTheta_ECLCROSS->push_back(extHit.getErrorTheta());
          m_errorPhi_ECLDL->push_back(-1);
          m_errorTheta_ECLDL->push_back(-1);
        } else if (extHit.getStatus() == EXT_ECLDL) {
          m_errorPhi_ECLNEAR->push_back(-1);
          m_errorTheta_ECLNEAR->push_back(-1);
          m_errorPhi_ECLCROSS->push_back(-1);
          m_errorTheta_ECLCROSS->push_back(-1);
          m_errorPhi_ECLDL->push_back(extHit.getErrorPhi());
          m_errorTheta_ECLDL->push_back(extHit.getErrorTheta());
        }
        // double errorPhi = extHit.getErrorPhi();
        // if (errorPhi > 2 * M_PI) continue;
        // m_errorPhi->push_back(errorPhi);
        // double errorTheta = extHit.getErrorTheta();
        // if (errorTheta > M_PI) continue;
        // m_errorTheta->push_back(errorTheta);
        double deltaPhi = extHit.getPosition().Phi() - eclCluster->getPhi();
        m_deltaPhi->push_back(deltaPhi);
        m_phiCluster->push_back(eclCluster->getPhi());
        double deltaTheta = extHit.getPosition().Theta() - eclCluster->getTheta();
        m_deltaTheta->push_back(deltaTheta);
        m_thetaCluster->push_back(eclCluster->getTheta());
        double quality = clusterQuality(extHit, deltaPhi, deltaTheta);
        m_quality->push_back(quality);
        if (quality < quality_tmp) {
          quality_tmp = quality;
          cluster_best = eclCluster;
          hitStatus = extHit.getStatus();
        }
        m_trackNo->push_back(i);
        m_trackMomentum->push_back(track.getTrackFitResult(Const::pion)->getMomentum().Mag());
      }
    } // end loop on ExtHits related to Track
    m_quality_best->push_back(quality_tmp);
    m_hitstatus_best->push_back(hitStatus);
    if (cluster_best != nullptr) {
      cluster_best->setIsTrack(true);
      track.addRelationTo(cluster_best);
    }
  } // end loop on Tracks
  m_tree->Fill();
}

void ECLTrackClusterMatchingModule::endRun()
{
}

void ECLTrackClusterMatchingModule::terminate()
{
  if (m_rootFilePtr != NULL) {
    m_rootFilePtr->cd();
    m_tree->Write();
    m_rootFilePtr->Close();
  }
}

bool ECLTrackClusterMatchingModule::isECLHit(const ExtHit& extHit) const
{
  if ((extHit.getDetectorID() != Const::EDetector::ECL)) return false;
  ExtHitStatus extHitStatus = extHit.getStatus();
  if (extHitStatus == EXT_ECLCROSS || extHitStatus == EXT_ECLDL || extHitStatus == EXT_ECLNEAR) return true;
  else return false;
}

double ECLTrackClusterMatchingModule::clusterQuality(const ExtHit& extHit, double deltaPhi, double deltaTheta) const
{
  return sqrt(deltaTheta * deltaTheta / (extHit.getErrorTheta() * extHit.getErrorTheta()) + deltaPhi * deltaPhi /
              (extHit.getErrorPhi() * extHit.getErrorPhi()));
}
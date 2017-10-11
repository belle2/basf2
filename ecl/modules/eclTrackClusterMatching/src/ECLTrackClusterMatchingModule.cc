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
    m_deltaPhi(0),
    m_deltaTheta(0),
    m_quality(0),
    m_quality_best(0)
{
  setDescription("Match Tracks to ECLCluster");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("writeToRoot", m_writeToRoot,
           "set true if you want to save the information in a root file named by parameter 'rootFileName'", bool(true));
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

  if (m_writeToRoot == true) {
    m_rootFilePtr = new TFile(m_rootFileName.c_str(), "RECREATE");
  } else
    m_rootFilePtr = NULL;

  TDirectory* oldDir = gDirectory;
  m_rootFilePtr->cd();
  // initialize tree
  m_tree     = new TTree("m_tree", "ECL Track Cluster Matching Analysis tree");
  oldDir->cd();

  m_tree->Branch("expNo", &m_iExperiment, "expNo/I");
  m_tree->Branch("runNo", &m_iRun, "runNo/I");
  m_tree->Branch("evtNo", &m_iEvent, "evtNo/I");

  m_tree->Branch("deltaPhi", "std::vector<double>",  &m_deltaPhi);
  m_tree->Branch("errorPhi", "std::vector<double>",  &m_errorPhi);
  m_tree->Branch("deltaTheta", "std::vector<double>",  &m_deltaTheta);
  m_tree->Branch("errorTheta", "std::vector<double>",  &m_errorTheta);
  m_tree->Branch("quality", "std::vector<double>",  &m_quality);
  m_tree->Branch("quality_best", "std::vector<double>",  &m_quality_best);

  B2INFO("[ECLTrackClusterMatchingModule]: Initialization of ECLTrackClusterMatching Module completed.");
}

void ECLTrackClusterMatchingModule::beginRun()
{
}

void ECLTrackClusterMatchingModule::event()
{
  m_deltaPhi->clear();
  m_errorPhi->clear();
  m_deltaTheta->clear();
  m_errorTheta->clear();
  m_quality->clear();
  m_quality_best->clear();

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

  for (const Track& track : tracks) {

    ECLCluster* cluster_best = nullptr;
    double quality_tmp = 1e6;
    // Find extrapolated track hits in the ECL, considering only hit points
    // that either are on the sphere, closest to or on radial direction of an
    // ECLCluster.
    for (const auto& extHit : track.getRelationsTo<ExtHit>()) {
      if (!isECLHit(extHit)) continue;
      double errorPhi = extHit.getErrorPhi();
      if (errorPhi > 2 * M_PI) continue;
      m_errorPhi->push_back(errorPhi);
      double errorTheta = extHit.getErrorTheta();
      if (errorTheta > M_PI) continue;
      m_errorTheta->push_back(errorTheta);
      ECLCluster* eclCluster = extHit.getRelatedFrom<ECLCluster>();
      double deltaPhi = extHit.getPosition().Phi() - eclCluster->getPhi();
      m_deltaPhi->push_back(deltaPhi);
      double deltaTheta = extHit.getPosition().Theta() - eclCluster->getTheta();
      m_deltaTheta->push_back(deltaTheta);
      double quality = clusterQuality(extHit, deltaPhi, deltaTheta);
      m_quality->push_back(quality);
      if (quality < quality_tmp) {
        quality_tmp = quality;
        cluster_best = eclCluster;
      }
    } // end loop on ExtHits related to Track
    m_quality_best->push_back(quality_tmp);
    if (cluster_best != nullptr) {
      if (cluster_best->isTrack()) {
        Track* previously_matching_track = cluster_best->getRelatedFrom<Track>();
        track.addRelationTo(previously_matching_track);
        previously_matching_track->addRelationTo(&track);
      } else cluster_best->setIsTrack(true);
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
  if (m_tree != NULL) {
    TDirectory* oldDir = gDirectory;
    if (m_rootFilePtr)
      m_rootFilePtr->cd();
    m_tree->Write();
    oldDir->cd();
  }
  if (m_rootFilePtr != NULL) {
    m_rootFilePtr->Close();
  }
}

bool ECLTrackClusterMatchingModule::checkPionECLEnterID(const ExtHit& extHit) const
{
  if (abs(extHit.getPdgCode()) != Const::pion.getPDGCode()) return true;
  else if ((extHit.getDetectorID() != Const::EDetector::ECL)) return true;
  else if (extHit.getStatus() != EXT_ENTER) return true;
  else if (extHit.getCopyID() == -1) return true;
  else return false;
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
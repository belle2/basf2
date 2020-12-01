/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jonas Wagner, Sebastian Racs                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/trackQualityEstimator/TrackQETrainingDataCollectorModule.h>

using namespace Belle2;


REG_MODULE(TrackQETrainingDataCollector)

TrackQETrainingDataCollectorModule::TrackQETrainingDataCollectorModule() : Module()
{
  //Set module properties
  setDescription("Module to collect training data for a specified qualityEstimator and store it in a root file.");
  setPropertyFlags(c_ParallelProcessingCertified | c_TerminateInAllProcesses);

  addParam("recoTracksStoreArrayName",
           m_recoTracksStoreArrayName,
           "Name of the recoTrack StoreArray.",
           m_recoTracksStoreArrayName);

  addParam("SVDCDCRecoTracksStoreArrayName",
           m_svdCDCRecoTracksStoreArrayName,
           "Name of the SVD-CDC StoreArray.",
           m_svdCDCRecoTracksStoreArrayName);

  addParam("SVDPlusCDCStandaloneRecoTracksStoreArrayName",
           m_svdPlusCDCStandaloneRecoTracksStoreArrayName,
           "Name of the combined CDC-SVD StoreArray with tracks added from the CDC to SVD CKF.",
           m_svdPlusCDCStandaloneRecoTracksStoreArrayName);

  addParam("CDCRecoTracksStoreArrayName",
           m_cdcRecoTracksStoreArrayName,
           "Name of the CDC StoreArray.",
           m_cdcRecoTracksStoreArrayName);

  addParam("SVDRecoTracksStoreArrayName",
           m_svdRecoTracksStoreArrayName,
           "Name of the SVD StoreArray.",
           m_svdRecoTracksStoreArrayName);

  addParam("PXDRecoTracksStoreArrayName",
           m_pxdRecoTracksStoreArrayName,
           "Name of the PXD StoreArray.",
           m_pxdRecoTracksStoreArrayName);

  addParam("TrainingDataOutputName",
           m_TrainingDataOutputName,
           "Name of the output rootfile.",
           m_TrainingDataOutputName);

  addParam("collectEventFeatures",
           m_collectEventFeatures,
           "Whether to use eventwise features.",
           m_collectEventFeatures);
}

void TrackQETrainingDataCollectorModule::initialize()
{
  m_recoTracks.isRequired(m_recoTracksStoreArrayName);
  if (m_collectEventFeatures) {
    m_eventInfoExtractor = std::make_unique<EventInfoExtractor>(m_variableSet);
  }
  m_recoTrackExtractor = std::make_unique<RecoTrackExtractor>(m_variableSet);
  m_subRecoTrackExtractor = std::make_unique<SubRecoTrackExtractor>(m_variableSet);
  m_hitInfoExtractor = std::make_unique<HitInfoExtractor>(m_variableSet);

  m_variableSet.emplace_back("truth", &m_matched);
  m_variableSet.emplace_back("background", &m_background);
  m_variableSet.emplace_back("ghost", &m_ghost);
  m_variableSet.emplace_back("fake", &m_fake);
  m_variableSet.emplace_back("clone", &m_clone);

  m_recorder = std::make_unique<SimpleVariableRecorder>(m_variableSet, m_TrainingDataOutputName, "tree");
}

void TrackQETrainingDataCollectorModule::beginRun()
{

}

void TrackQETrainingDataCollectorModule::event()
{
  for (const RecoTrack& recoTrack : m_recoTracks) {
    m_matched = float(recoTrack.getMatchingStatus() ==  RecoTrack::MatchingStatus::c_matched);
    m_background = float(recoTrack.getMatchingStatus() ==  RecoTrack::MatchingStatus::c_background);
    m_ghost = float(recoTrack.getMatchingStatus() ==  RecoTrack::MatchingStatus::c_ghost);
    m_fake = float((recoTrack.getMatchingStatus() ==  RecoTrack::MatchingStatus::c_background)
                   || (recoTrack.getMatchingStatus() ==  RecoTrack::MatchingStatus::c_ghost));
    m_clone = float(recoTrack.getMatchingStatus() ==  RecoTrack::MatchingStatus::c_clone);

    RecoTrack* pxdRecoTrackPtr = recoTrack.getRelatedTo<RecoTrack>(m_pxdRecoTracksStoreArrayName);
    // combined CDC and SVD tracks after both CDC-to-SVD and also SVD-to-CDC CKF
    RecoTrack* svdCDCRecoTrackPtr = recoTrack.getRelatedTo<RecoTrack>(m_svdCDCRecoTracksStoreArrayName);
    // combined SVD and CDC-standalone tracks after CDC-to-SVD CKF
    RecoTrack* svdPlusCDCStandaloneRecoTrackPtr = nullptr;
    // CDC tracks from CDC-standalone tracking
    RecoTrack* cdcRecoTrackPtr = nullptr;
    // SVD tracks from VXDTF2 (SVD-standalone) tracking
    RecoTrack* svdRecoTrackPtr = nullptr;

    if (svdCDCRecoTrackPtr) {
      // Relation graph when SVD-to-CDC CFK is enabled:
      // SVDCDCRecoTracks -> SVDPlusCDCStandaloneRecoTracks -> CDCRecoTracks & SVDRecoTracks
      svdPlusCDCStandaloneRecoTrackPtr = svdCDCRecoTrackPtr->getRelatedTo<RecoTrack>(
                                           m_svdPlusCDCStandaloneRecoTracksStoreArrayName);
      if (not svdPlusCDCStandaloneRecoTrackPtr) {
        // Relation graph when SVD-to-CDC CFK is disabled:
        // SVDCDCRecoTracks -> CDCRecoTracks & SVDRecoTracks
        svdPlusCDCStandaloneRecoTrackPtr = svdCDCRecoTrackPtr;
      }
      cdcRecoTrackPtr = svdPlusCDCStandaloneRecoTrackPtr->getRelatedTo<RecoTrack>(m_cdcRecoTracksStoreArrayName);
      svdRecoTrackPtr = svdPlusCDCStandaloneRecoTrackPtr->getRelatedTo<RecoTrack>(m_svdRecoTracksStoreArrayName);
    }

    if (m_collectEventFeatures) {
      m_eventInfoExtractor->extractVariables(m_recoTracks, recoTrack);
    }
    m_recoTrackExtractor->extractVariables(recoTrack);
    // TODO: also use `CKFCDCRecoTracks` and its features in quality estimation
    m_subRecoTrackExtractor->extractVariables(cdcRecoTrackPtr, svdRecoTrackPtr, pxdRecoTrackPtr);
    m_hitInfoExtractor->extractVariables(recoTrack);

    // record variables
    m_recorder->record();
  }
}

void TrackQETrainingDataCollectorModule::terminate()
{
  m_recorder->write();
  m_recorder.reset();
}

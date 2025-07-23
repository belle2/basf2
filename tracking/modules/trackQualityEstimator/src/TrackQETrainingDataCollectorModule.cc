/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/modules/trackQualityEstimator/TrackQETrainingDataCollectorModule.h>

using namespace Belle2;


REG_MODULE(TrackQETrainingDataCollector);

TrackQETrainingDataCollectorModule::TrackQETrainingDataCollectorModule() : Module()
{
  //Set module properties
  setDescription("Module to collect training data for a specified qualityEstimator and store it in a root file.");
  setPropertyFlags(c_ParallelProcessingCertified | c_TerminateInAllProcesses);

  m_cdcRecoTracksStoreArrayBacktrackChain = std::vector<std::string>();
  m_svdRecoTracksStoreArrayBacktrackChain = std::vector<std::string>();
  addParam("recoTracksStoreArrayName",
           m_recoTracksStoreArrayName,
           "Name of the recoTrack StoreArray.",
           m_recoTracksStoreArrayName);

  addParam("cdcRecoTracksStoreArrayBacktrackChain",
           m_cdcRecoTracksStoreArrayBacktrackChain,
           "Backtrack Chain for finding the CDC StoreArray.",
           m_cdcRecoTracksStoreArrayBacktrackChain);

  addParam("svdRecoTracksStoreArrayBacktrackChain",
           m_svdRecoTracksStoreArrayBacktrackChain,
           "Backtrack Chain for finding the SVD StoreArray.",
           m_svdRecoTracksStoreArrayBacktrackChain);

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

    // CDC tracks from CDC-standalone tracking
    const RecoTrack* cdcRecoTrackPtr{&recoTrack};
    int length = m_cdcRecoTracksStoreArrayBacktrackChain.size();
    // The last item must be "RecoTrack", so begin with the second
    for (int i = length - 2; i >= 0; i--) {
      std::string& name = m_cdcRecoTracksStoreArrayBacktrackChain[i];
      cdcRecoTrackPtr = cdcRecoTrackPtr->getRelatedTo<RecoTrack>(name);
      if (!cdcRecoTrackPtr) {
        break;
      }
    }

    // SVD tracks from VXDTF2 (SVD-standalone) tracking
    const RecoTrack* svdRecoTrackPtr{&recoTrack};
    length = m_svdRecoTracksStoreArrayBacktrackChain.size();
    // The last item must be "RecoTrack", so begin with the second
    for (int i = length - 2; i >= 0; i--) {
      std::string& name = m_svdRecoTracksStoreArrayBacktrackChain[i];
      svdRecoTrackPtr = svdRecoTrackPtr->getRelatedTo<RecoTrack>(name);
      if (!svdRecoTrackPtr) {
        break;
      }
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

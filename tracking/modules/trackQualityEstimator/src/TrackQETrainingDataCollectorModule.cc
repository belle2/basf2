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

  addParam("recoTracksStoreArrayName", m_recoTracksStoreArrayName, "Name of the recoTrack StoreArray.",
           std::string("RecoTracks"));
  addParam("SVDCDCRecoTracksStoreArrayName", m_svdCDCRecoTracksStoreArrayName , "Name of the SVD-CDC StoreArray.",
           std::string("SVDCDCRecoTracks"));
  addParam("SVDPlusCDCStandaloneRecoTracksStoreArrayName", m_svdPlusCDCStandaloneRecoTracksStoreArrayName ,
           "Name of the SVD StoreArray with tracks added from the CDC to SVD CKF.",
           std::string("SVDplusRecoTracks"));
  addParam("CDCRecoTracksStoreArrayName", m_cdcRecoTracksStoreArrayName , "Name of the CDC StoreArray.",
           std::string("CDCRecoTracks"));
  addParam("SVDRecoTracksStoreArrayName", m_svdRecoTracksStoreArrayName , "Name of the SVD StoreArray.",
           std::string("SVDRecoTracks"));
  addParam("PXDRecoTracksStoreArrayName", m_pxdRecoTracksStoreArrayName , "Name of the PXD StoreArray.",
           std::string("PXDRecoTracks"));
  addParam("TrainingDataOutputName", m_TrainingDataOutputName, "Name of the output rootfile.",
           std::string("QETrainingOutput.root"));
  addParam("collectEventFeatures", m_param_collectEventFeatures, "Whether to use eventwise features.",
           false);
}

void TrackQETrainingDataCollectorModule::initialize()
{
  m_recoTracks.isRequired(m_recoTracksStoreArrayName);
  if (m_param_collectEventFeatures) {
    m_eventInfoExtractor = std::make_unique<EventInfoExtractor>(m_variableSet);
  }
  m_recoTrackExtractor = std::make_unique<RecoTrackExtractor>(m_variableSet);
  m_subRecoTrackExtractor = std::make_unique<SubRecoTrackExtractor>(m_variableSet);
  m_hitInfoExtractor = std::make_unique<HitInfoExtractor>(m_variableSet);

  m_variableSet.emplace_back("truth", &m_truth);

  m_recorder = std::make_unique<SimpleVariableRecorder>(m_variableSet, m_TrainingDataOutputName, "tree");
}

void TrackQETrainingDataCollectorModule::beginRun()
{

}

void TrackQETrainingDataCollectorModule::event()
{
  for (const RecoTrack& recoTrack : m_recoTracks) {
    m_truth = float(recoTrack.getMatchingStatus() ==  RecoTrack::MatchingStatus::c_matched);

    RecoTrack* pxdRecoTrackPtr = recoTrack.getRelatedTo<RecoTrack>(m_pxdRecoTracksStoreArrayName);
    // track after both CDC-to-SVD and also SVD-to-CDC CKF if it is enabled
    RecoTrack* svdCDCRecoTrackPtr = recoTrack.getRelatedTo<RecoTrack>(m_svdCDCRecoTracksStoreArrayName);
    // tracks after CDC-to-SVD CKF
    RecoTrack* svdPlusCDCStandaloneRecoTrackPtr = nullptr;
    RecoTrack* cdcRecoTrackPtr = nullptr;
    RecoTrack* svdRecoTrackPtr = nullptr;

    if (svdCDCRecoTrackPtr) {
      svdPlusCDCStandaloneRecoTrackPtr =
        svdCDCRecoTrackPtr->getRelatedTo<RecoTrack>(m_svdPlusCDCStandaloneRecoTracksStoreArrayName);
      // When `use_svd_to_cdc_ckf` is false, there is no related intermediate
      // svdPlusCDCStandalone store array, and the svdCDC store array relates
      // directly to CDC and SVD tracks. Then, set the svdPlusCDCStandalone
      // pointer to be equal to the svdCDC pointer.
      if (not svdPlusCDCStandaloneRecoTrackPtr) {
        svdPlusCDCStandaloneRecoTrackPtr = svdCDCRecoTrackPtr;
      }
    }
    if (svdPlusCDCStandaloneRecoTrackPtr) {
      // TODO: also use `CKFCDCRecoTracks` in quality estimation
      cdcRecoTrackPtr = svdCDCRecoTrackPtr->getRelatedTo<RecoTrack>(m_cdcRecoTracksStoreArrayName);
      svdRecoTrackPtr = svdCDCRecoTrackPtr->getRelatedTo<RecoTrack>(m_svdRecoTracksStoreArrayName);
    }
    if (m_param_collectEventFeatures) {
      m_eventInfoExtractor->extractVariables(m_recoTracks, recoTrack);
    }
    m_recoTrackExtractor->extractVariables(recoTrack);
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

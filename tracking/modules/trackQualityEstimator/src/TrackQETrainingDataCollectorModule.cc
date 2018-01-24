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
  addParam("SVDCDCRecoTracksStoreArrayName", m_svdcdcRecoTracksStoreArrayName , "Name of the SVD-CDC StoreArray.",
           std::string("SVDCDCRecoTracks"));
  addParam("CDCRecoTracksStoreArrayName", m_cdcRecoTracksStoreArrayName , "Name of the CDC StoreArray.",
           std::string("CDCRecoTracks"));
  addParam("SVDRecoTracksStoreArrayName", m_svdRecoTracksStoreArrayName , "Name of the SVD StoreArray.",
           std::string("SVDRecoTracks"));
  addParam("PXDRecoTracksStoreArrayName", m_pxdRecoTracksStoreArrayName , "Name of the PXD StoreArray.",
           std::string("PXDRecoTracks"));

  addParam("TrainingDataOutputName", m_TrainingDataOutputName, "Name of the output rootfile.", std::string("QETrainingOutput.root"));

}

void TrackQETrainingDataCollectorModule::initialize()
{
  m_recoTracks.isRequired(m_recoTracksStoreArrayName);

  m_eventInfoExtractor = std::make_unique<EventInfoExtractor>(m_variableSet);
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

    RecoTrack* svdcdcRecoTrack; RecoTrack* cdcRecoTrack; RecoTrack* svdRecoTrack; RecoTrack* pxdRecoTrack;

    svdcdcRecoTrack = recoTrack.getRelatedTo<RecoTrack>(m_svdcdcRecoTracksStoreArrayName);
    if (svdcdcRecoTrack) {
      cdcRecoTrack = svdcdcRecoTrack->getRelatedTo<RecoTrack>(m_cdcRecoTracksStoreArrayName);
      svdRecoTrack = svdcdcRecoTrack->getRelatedTo<RecoTrack>(m_svdRecoTracksStoreArrayName);
    }
    pxdRecoTrack = recoTrack.getRelatedTo<RecoTrack>(m_pxdRecoTracksStoreArrayName);

    m_eventInfoExtractor->extractVariables(m_recoTracks, recoTrack);
    m_recoTrackExtractor->extractVariables(recoTrack);
    m_subRecoTrackExtractor->extractVariables(cdcRecoTrack, svdRecoTrack, pxdRecoTrack);
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

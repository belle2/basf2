/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/modules/trackQualityEstimator/TrackQualityEstimatorMVAModule.h>

using namespace Belle2;


REG_MODULE(TrackQualityEstimatorMVA);

TrackQualityEstimatorMVAModule::TrackQualityEstimatorMVAModule() : Module()
{
  //Set module properties
  setDescription("The quality estimator module for a fully reconstructed track");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("recoTracksStoreArrayName",
           m_recoTracksStoreArrayName,
           "Name of the recoTrack StoreArray.",
           m_recoTracksStoreArrayName);

  addParam("SVDCDCRecoTracksStoreArrayName",
           m_svdcdcRecoTracksStoreArrayName,
           "Name of the SVD-CDC StoreArray.",
           m_svdcdcRecoTracksStoreArrayName);

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

  addParam("TracksStoreArrayName",
           m_tracksStoreArrayName,
           "Name of the fitted mdst Tracks StoreArray.",
           m_tracksStoreArrayName);

  addParam("WeightFileIdentifier",
           m_weightFileIdentifier,
           "Identifier of weightfile in Database or local root/xml file.",
           m_weightFileIdentifier);

  addParam("collectEventFeatures",
           m_collectEventFeatures,
           "Whether to use eventwise features.",
           m_collectEventFeatures);
}

void TrackQualityEstimatorMVAModule::initialize()
{
  m_recoTracks.isRequired(m_recoTracksStoreArrayName);

  if (m_collectEventFeatures) {
    m_eventInfoExtractor = std::make_unique<EventInfoExtractor>(m_variableSet);
  }
  m_recoTrackExtractor = std::make_unique<RecoTrackExtractor>(m_variableSet);
  m_subRecoTrackExtractor = std::make_unique<SubRecoTrackExtractor>(m_variableSet);
  m_hitInfoExtractor = std::make_unique<HitInfoExtractor>(m_variableSet);

  m_mvaExpert = std::make_unique<MVAExpert>(m_weightFileIdentifier, m_variableSet);
  m_mvaExpert->initialize();
}

void TrackQualityEstimatorMVAModule::beginRun()
{
  m_mvaExpert->beginRun();
}

void TrackQualityEstimatorMVAModule::event()
{
  for (RecoTrack& recoTrack : m_recoTracks) {
    const RecoTrack* pxdRecoTrack = recoTrack.getRelatedTo<RecoTrack>(m_pxdRecoTracksStoreArrayName);
    const RecoTrack* svdcdcRecoTrack =
      recoTrack.getRelatedTo<RecoTrack>(m_svdcdcRecoTracksStoreArrayName);
    RecoTrack* cdcRecoTrack{nullptr};
    RecoTrack* svdRecoTrack{nullptr};
    if (svdcdcRecoTrack) {
      cdcRecoTrack = svdcdcRecoTrack->getRelatedTo<RecoTrack>(m_cdcRecoTracksStoreArrayName);
      svdRecoTrack = svdcdcRecoTrack->getRelatedTo<RecoTrack>(m_svdRecoTracksStoreArrayName);
    }

    if (m_collectEventFeatures) {
      m_eventInfoExtractor->extractVariables(m_recoTracks, recoTrack);
    }
    m_recoTrackExtractor->extractVariables(recoTrack);
    m_subRecoTrackExtractor->extractVariables(cdcRecoTrack, svdRecoTrack, pxdRecoTrack);
    m_hitInfoExtractor->extractVariables(recoTrack);
    // get quality indicator from classifier
    const float qualityIndicator = m_mvaExpert->predict();
    // set quality indicator property in RecoTracks and mdst Tracks from track fit
    recoTrack.setQualityIndicator(qualityIndicator);
  }
}

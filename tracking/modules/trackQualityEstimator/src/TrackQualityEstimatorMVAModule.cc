/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sebastian Racs, Michael Eliachevitch                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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

  addParam("WeightFileIdentifier",
           m_WeightFileIdentifier,
           "Identifier of weightfile in Database or local root/xml file.",
           m_WeightFileIdentifier);

  addParam("collectEventFeatures",
           m_param_collectEventFeatures,
           "Whether to use eventwise features.",
           m_param_collectEventFeatures);
}

void TrackQualityEstimatorMVAModule::initialize()
{
  m_recoTracks.isRequired(m_recoTracksStoreArrayName);

  if (m_param_collectEventFeatures) {
    m_eventInfoExtractor = std::make_unique<EventInfoExtractor>(m_variableSet);
  }
  m_recoTrackExtractor = std::make_unique<RecoTrackExtractor>(m_variableSet);
  m_subRecoTrackExtractor = std::make_unique<SubRecoTrackExtractor>(m_variableSet);
  m_hitInfoExtractor = std::make_unique<HitInfoExtractor>(m_variableSet);

  m_mvaExpert = std::make_unique<MVAExpert>(m_WeightFileIdentifier, m_variableSet);
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
    RecoTrack* cdcRecoTrack;
    RecoTrack* svdRecoTrack;
    if (svdcdcRecoTrack) {
      cdcRecoTrack = svdcdcRecoTrack->getRelatedTo<RecoTrack>(m_cdcRecoTracksStoreArrayName);
      svdRecoTrack = svdcdcRecoTrack->getRelatedTo<RecoTrack>(m_svdRecoTracksStoreArrayName);
    }

    if (m_param_collectEventFeatures) {
      m_eventInfoExtractor->extractVariables(m_recoTracks, recoTrack);
    }
    m_recoTrackExtractor->extractVariables(recoTrack);
    m_subRecoTrackExtractor->extractVariables(cdcRecoTrack, svdRecoTrack, pxdRecoTrack);
    m_hitInfoExtractor->extractVariables(recoTrack);

    const float qualityIndicator = m_mvaExpert->predict();
    recoTrack.setQualityIndicator(qualityIndicator);
  }
}

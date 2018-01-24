/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sebastian Racs                                           *
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

  addParam("WeightFileIdentifier", m_WeightFileIdentifier,
           "Identifier of weightfile in Database or local root/xml file.", std::string(""));
}

void TrackQualityEstimatorMVAModule::initialize()
{
  m_recoTracks.isRequired(m_recoTracksStoreArrayName);

  m_eventInfoExtractor = std::make_unique<EventInfoExtractor>(m_variableSet);
  m_recoTrackExtractor = std::make_unique<RecoTrackExtractor>(m_variableSet);
  m_subRecoTrackExtractor = std::make_unique<SubRecoTrackExtractor>(m_variableSet);
  m_hitInfoExtractor = std::make_unique<HitInfoExtractor>(m_variableSet);

  m_variableSet.emplace_back("N_RecoTracks", &m_nRecoTracks);

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
    m_nRecoTracks = m_recoTracks.getEntries();

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

    float qi = m_mvaExpert->predict();
    recoTrack.setQualityIndicator(qi);
  }


}

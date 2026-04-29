/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/modules/trackQualityEstimator/TrackQETrainingDataCollectorModule.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorTripletFit.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorCircleFit.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorRiemannHelixFit.h>

using namespace Belle2;


REG_MODULE(TrackQETrainingDataCollector);

TrackQETrainingDataCollectorModule::TrackQETrainingDataCollectorModule() : Module()
{
  //Set module properties
  setDescription("Module to collect training data for a specified qualityEstimator and store it in a root file.");
  setPropertyFlags(c_ParallelProcessingCertified | c_TerminateInAllProcesses);

  addParam("recoTracksStoreArrayName",
           m_recoTracksStoreArrayName,
           "Name of the recoTrack StoreArray.",
           m_recoTracksStoreArrayName);

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

  addParam("SVDEstimationMethod",
           m_SVDEstimationMethod,
           "Identifier which estimation method to use for SVD. Valid identifiers are: [tripletFit, circleFit, helixFit]",
           m_SVDEstimationMethod);
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

  // create pointer to chosen estimator
  if (m_SVDEstimationMethod == "tripletFit") {
    m_estimator = std::make_unique<QualityEstimatorTripletFit>();
  } else if (m_SVDEstimationMethod == "circleFit") {
    m_estimator = std::make_unique<QualityEstimatorCircleFit>();
  } else if (m_SVDEstimationMethod == "helixFit") {
    m_estimator = std::make_unique<QualityEstimatorRiemannHelixFit>();
  }
  B2ASSERT("QualityEstimator could not be initialized with method: " << m_SVDEstimationMethod, m_estimator);

  m_qeResultsExtractor = std::make_unique<QEResultsExtractor>(m_SVDEstimationMethod, m_variableSet, "SVD_");
  m_variableSet.emplace_back("SVD_NSpacePoints", &m_nSpacePoints);
  m_clusterInfoExtractor = std::make_unique<ClusterInfoExtractor>(m_variableSet, false, "SVD_");

  m_qeResultsExtractorBefore = std::make_unique<QEResultsExtractor>(m_SVDEstimationMethod, m_variableSet, "SVDbefore_");
  m_variableSet.emplace_back("SVDbefore_NSpacePoints", &m_nSpacePointsBefore);
  m_clusterInfoExtractorBefore = std::make_unique<ClusterInfoExtractor>(m_variableSet, false, "SVDbefore_");

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


    // Try to find all CDC tracks that are related to some hits in the RecoTrack.
    std::vector<RecoTrack*> allCDCTracks;
    const auto& cdcHitList = recoTrack.getCDCHitList();
    for (auto* cdcHit : cdcHitList) {
      const RelationVector<RecoTrack>& relatedCDCTracks =
        cdcHit->getRelationsWith<RecoTrack>(m_cdcRecoTracksStoreArrayName);
      for (unsigned int index = 0; index < relatedCDCTracks.size(); ++index) {
        RecoTrack* relatedCDCTrack = relatedCDCTracks[index];
        if (std::find(allCDCTracks.begin(), allCDCTracks.end(), relatedCDCTrack) == allCDCTracks.end()) {
          allCDCTracks.push_back(relatedCDCTrack);
        }
      }
    }
    // The reconstructed track contains at most one CDC part.
    // Try to match the hit list to find the right CDC track.
    // If no matching CDC tracks are found, then cdcRecoTrackPtr will still be nullptr.
    RecoTrack* cdcRecoTrackPtr = nullptr;
    for (RecoTrack* foundCDCTrack : allCDCTracks) {
      const auto& foundCDCTrackHitList = foundCDCTrack->getCDCHitList();
      if (foundCDCTrackHitList.size() == cdcHitList.size() and
          std::equal(foundCDCTrackHitList.begin(), foundCDCTrackHitList.end(), cdcHitList.begin())) {
        cdcRecoTrackPtr = foundCDCTrack;
        break;
      }
    }

    // Try to find all SVD tracks that are related to some hits in the RecoTrack.
    std::vector<RecoTrack*> allSVDTracks;
    const auto& svdHitList = recoTrack.getSVDHitList();
    for (auto* svdHit : svdHitList) {
      const RelationVector<RecoTrack>& relatedSVDTracks = svdHit->getRelationsWith<RecoTrack>(m_svdRecoTracksStoreArrayName);
      for (unsigned int index = 0; index < relatedSVDTracks.size(); ++index) {
        RecoTrack* relatedSVDTrack = relatedSVDTracks[index];
        if (std::find(allSVDTracks.begin(), allSVDTracks.end(), relatedSVDTrack) == allSVDTracks.end()) {
          allSVDTracks.push_back(relatedSVDTrack);
        }
      }
    }
    // The reconstructed track contains at most two SVD parts.
    // Try to match the hit list to find the right SVD tracks.
    // If no matching SVD tracks are found, then svdRecoTrackPtr will still be nullptr.
    RecoTrack* svdRecoTrackPtr = nullptr;
    RecoTrack* svdRecoTrackBeforePtr = nullptr;
    RecoTrack* svdRecoTrackAfterPtr = nullptr;
    // First try to match the whole SVD track, which means it only contains one SVD part.
    for (RecoTrack* foundSVDTrack : allSVDTracks) {
      const auto& foundSVDTrackHitList = foundSVDTrack->getSVDHitList();
      if (foundSVDTrackHitList.size() != svdHitList.size())
        continue;
      if (std::equal(foundSVDTrackHitList.begin(), foundSVDTrackHitList.end(), svdHitList.begin())) {
        svdRecoTrackPtr = foundSVDTrack;
        break;
      }
    }
    if (svdRecoTrackPtr == nullptr) {
      // Next try to match two SVD tracks.
      for (RecoTrack* foundSVDTrackBefore : allSVDTracks) {
        const auto& foundSVDTrackBeforeHitList = foundSVDTrackBefore->getSVDHitList();
        int sizeBefore = foundSVDTrackBeforeHitList.size();
        if (sizeBefore >= svdHitList.size())
          continue;
        if (not std::equal(foundSVDTrackBeforeHitList.begin(), foundSVDTrackBeforeHitList.end(), svdHitList.begin()))
          continue;
        int rest = svdHitList.size() - sizeBefore;
        for (RecoTrack* foundSVDTrackAfter : allSVDTracks) {
          const auto& foundSVDTrackAfterHitList = foundSVDTrackAfter->getSVDHitList();
          if (foundSVDTrackBefore == foundSVDTrackAfter)
            continue;
          if (foundSVDTrackAfterHitList.size() != rest)
            continue;
          if (std::equal(foundSVDTrackAfterHitList.begin(), foundSVDTrackAfterHitList.end(), svdHitList.begin() + sizeBefore)) {
            svdRecoTrackPtr = foundSVDTrackAfter;
            svdRecoTrackBeforePtr = foundSVDTrackBefore;
            svdRecoTrackAfterPtr = foundSVDTrackAfter;
            break;
          }
        }
        if (svdRecoTrackBeforePtr)
          break;
      }
    }

    const SpacePointTrackCand* spacePointTrackCand = nullptr;
    if (svdRecoTrackPtr) {
      spacePointTrackCand = svdRecoTrackPtr->getRelatedTo<SpacePointTrackCand>("SPTrackCands");
    }
    std::vector<SpacePoint const*> sortedHits;
    if (spacePointTrackCand and spacePointTrackCand->hasRefereeStatus(SpacePointTrackCand::c_isActive)) {
      sortedHits = spacePointTrackCand->getSortedHits();
    }

    m_clusterInfoExtractor->extractVariables(sortedHits);
    m_nSpacePoints = sortedHits.size();
    m_qeResultsExtractor->extractVariables(m_estimator->estimateQualityAndProperties(sortedHits));

    const SpacePointTrackCand* spacePointTrackCandBefore = nullptr;
    if (svdRecoTrackBeforePtr) {
      spacePointTrackCandBefore = svdRecoTrackBeforePtr->getRelatedTo<SpacePointTrackCand>("SPTrackCands");
    }
    std::vector<SpacePoint const*> sortedHitsBefore;
    if (spacePointTrackCandBefore and spacePointTrackCandBefore->hasRefereeStatus(SpacePointTrackCand::c_isActive)) {
      sortedHitsBefore = spacePointTrackCandBefore->getSortedHits();
    }

    m_clusterInfoExtractorBefore->extractVariables(sortedHitsBefore);
    m_nSpacePointsBefore = sortedHits.size();
    m_qeResultsExtractorBefore->extractVariables(m_estimator->estimateQualityAndProperties(sortedHitsBefore));

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

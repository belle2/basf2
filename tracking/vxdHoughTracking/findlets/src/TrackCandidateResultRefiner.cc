/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/vxdHoughTracking/findlets/TrackCandidateResultRefiner.h>
#include <framework/core/ModuleParamList.h>
#include <framework/core/ModuleParamList.templateDetails.h>
#include <framework/geometry/BFieldManager.h>
#include <tracking/spacePointCreation/SpacePointTrackCand.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorCircleFit.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorMC.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorRiemannHelixFit.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorTripletFit.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

using namespace Belle2;
using namespace TrackFindingCDC;
using namespace vxdHoughTracking;

TrackCandidateResultRefiner::~TrackCandidateResultRefiner() = default;


TrackCandidateResultRefiner::TrackCandidateResultRefiner() : Super()
{
  addProcessingSignalListener(&m_overlapResolver);
}

void TrackCandidateResultRefiner::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  Super::exposeParameters(moduleParamList, prefix);

  m_overlapResolver.exposeParameters(moduleParamList, TrackFindingCDC::prefixed(prefix, "refinerOverlapResolver"));

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "trackQualityEstimationMethod"), m_EstimationMethod,
                                "Identifier which estimation method to use. Valid identifiers are: [mcInfo, circleFit, tripletFit, helixFit]",
                                m_EstimationMethod);
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "MCRecoTracksStoreArrayName"), m_MCRecoTracksStoreArrayName,
                                "Only required for MCInfo method. Name of StoreArray containing MCRecoTracks.",
                                m_MCRecoTracksStoreArrayName);
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "MCStrictQualityEstimator"), m_MCStrictQualityEstimator,
                                "Only required for MCInfo method. If false combining several MCTracks is allowed.",
                                m_MCStrictQualityEstimator);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "minQualitiyIndicatorSize3"), m_minQualitiyIndicatorSize3,
                                "Cut on quality indicator value for track candidates of size 3. Only accept SpacePointTrackCands with QI above this value.",
                                m_minQualitiyIndicatorSize3);
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "minQualitiyIndicatorSize4"), m_minQualitiyIndicatorSize4,
                                "Cut on quality indicator value for track candidates of size 4. Only accept SpacePointTrackCands with QI above this value.",
                                m_minQualitiyIndicatorSize4);
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "minQualitiyIndicatorSize5"), m_minQualitiyIndicatorSize5,
                                "Cut on quality indicator value for track candidates of size 5. Only accept SpacePointTrackCands with QI above this value.",
                                m_minQualitiyIndicatorSize5);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "maxNumberOfEachPathLength"), m_maxNumberOfEachPathLength,
                                "Maximum number of SpacePointTrackCands with a length of 3, 4, 5, or 6 each.",
                                m_maxNumberOfEachPathLength);
}

void TrackCandidateResultRefiner::initialize()
{
  Super::initialize();

  // create pointer to chosen estimator
  if (m_EstimationMethod == "mcInfo") {
    StoreArray<RecoTrack> mcRecoTracks;
    mcRecoTracks.isRequired(m_MCRecoTracksStoreArrayName);
    m_estimator = std::make_unique<QualityEstimatorMC>(m_MCRecoTracksStoreArrayName, m_MCStrictQualityEstimator);
  } else if (m_EstimationMethod == "tripletFit") {
    m_estimator = std::make_unique<QualityEstimatorTripletFit>();
  } else if (m_EstimationMethod == "circleFit") {
    m_estimator = std::make_unique<QualityEstimatorCircleFit>();
  } else if (m_EstimationMethod == "helixFit") {
    m_estimator = std::make_unique<QualityEstimatorRiemannHelixFit>();
  }
  B2ASSERT("QualityEstimator could not be initialized with method: " << m_EstimationMethod, m_estimator);
}

void TrackCandidateResultRefiner::beginRun()
{
  Super::beginRun();

  // BField is required by all QualityEstimators
  double bFieldZ = BFieldManager::getField(0, 0, 0).Z() / Unit::T;
  m_estimator->setMagneticFieldStrength(bFieldZ);

  if (m_EstimationMethod == "mcInfo") {
    QualityEstimatorMC* MCestimator = static_cast<QualityEstimatorMC*>(m_estimator.get());
    MCestimator->forceUpdateClusterNames();
  }
}

void TrackCandidateResultRefiner::apply(std::vector<SpacePointTrackCand>& unrefinedResults,
                                        std::vector<SpacePointTrackCand>& refinedResults)
{
  refinedResults.clear();
  std::vector<SpacePointTrackCand> selectedResults;
  selectedResults.reserve(unrefinedResults.size());
  // assign a QI computed using the selected QualityEstimator for each given SpacePointTrackCand
  for (SpacePointTrackCand& aTrackCandidate : unrefinedResults) {
    double qi = m_estimator->estimateQuality(aTrackCandidate.getSortedHits());
    aTrackCandidate.setQualityIndicator(qi);

    // Track candidates of size >= 6 are very rare. If the track candidate already is quite long (>= 6 hits),
    // it's very likely it's a valid track anyway, so QI is not checked.
    if ((aTrackCandidate.getNHits() == 3 and qi >= m_minQualitiyIndicatorSize3) or
        (aTrackCandidate.getNHits() == 4 and qi >= m_minQualitiyIndicatorSize4) or
        (aTrackCandidate.getNHits() == 5 and qi >= m_minQualitiyIndicatorSize5) or
        (aTrackCandidate.getNHits() >= 6))  {
      selectedResults.emplace_back(aTrackCandidate);
    }
  }

  // return early if nothing to do
  if (selectedResults.size() <= 1) {
    std::swap(selectedResults, refinedResults);
    return;
  }

  // sort by number of hits in the track candidate and by the QI
  std::sort(selectedResults.begin(), selectedResults.end(),
  [](const SpacePointTrackCand & a, const SpacePointTrackCand & b) {
    return ((a.getNHits() > b.getNHits()) or
            (a.getNHits() == b.getNHits() and a.getQualityIndicator() > b.getQualityIndicator()));
  });

  std::array<uint, 8> numberOfHitsInCheckedSPTCs{{0, 0, 0, 0, 0, 0, 0, 0}};
  refinedResults.reserve(selectedResults.size());
  for (auto& currentSPTC : selectedResults) {
    if (numberOfHitsInCheckedSPTCs[currentSPTC.size()] < m_maxNumberOfEachPathLength) {
      numberOfHitsInCheckedSPTCs[currentSPTC.size()] += 1;
      refinedResults.emplace_back(currentSPTC);
    }
  }

  m_overlapResolver.apply(refinedResults);
}

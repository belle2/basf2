/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Wessel                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/datcon/optimizedDATCON/findlets/TrackCandidateResultRefiner.h>

#include <framework/core/ModuleParamList.h>
#include <framework/core/ModuleParamList.templateDetails.h>
#include <framework/geometry/BFieldManager.h>

// #include <tracking/spacePointCreation/SpacePoint.h>
#include <tracking/spacePointCreation/SpacePointTrackCand.h>
#include <vxd/dataobjects/VxdID.h>
#include <vxd/geometry/GeoCache.h>

#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorCircleFit.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorMC.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorRiemannHelixFit.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorTripletFit.h>

#include <tracking/trackFindingCDC/filters/base/RelationFilterUtil.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <tracking/trackFindingCDC/utilities/Algorithms.h>
#include <tracking/trackFindingCDC/utilities/WeightedRelation.h>

#include <iostream>

using namespace Belle2;
using namespace TrackFindingCDC;

TrackCandidateResultRefiner::~TrackCandidateResultRefiner() = default;


TrackCandidateResultRefiner::TrackCandidateResultRefiner() : Super()
{
  // create pointer to chosen estimator
  // get SegFault if I do this in initialize, Why, I don't know. Likely a bug in the parent class
  if (m_EstimationMethod == "mcInfo") {
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

void TrackCandidateResultRefiner::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  Super::exposeParameters(moduleParamList, prefix);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "trackQualityEstimationMethod"), m_EstimationMethod,
                                "Identifier which estimation method to use. Valid identifiers are: [mcInfo, circleFit, tripletFit, helixFit]",
                                m_EstimationMethod);
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "MCRecoTracksStoreArrayName"), m_MCRecoTracksStoreArrayName,
                                "Only required for MCInfo method. Name of StoreArray containing MCRecoTracks.",
                                m_MCRecoTracksStoreArrayName);
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "MCStrictQualityEstimator"), m_MCStrictQualityEstimator,
                                "Only required for MCInfo method. If false combining several MCTracks is allowed.",
                                m_MCStrictQualityEstimator);
}

void TrackCandidateResultRefiner::initialize()
{
  Super::initialize();
}

void TrackCandidateResultRefiner::beginRun()
{
  Super::beginRun();

  // BField is required by all QualityEstimators
  double bFieldZ = BFieldManager::getField(0, 0, 0).Z() / Unit::T;
  m_estimator->setMagneticFieldStrength(bFieldZ);

  if (m_EstimationMethod == "mcInfo") {
    StoreArray<RecoTrack> mcRecoTracks;
    mcRecoTracks.isRequired(m_MCRecoTracksStoreArrayName);
    std::string svdClustersName = ""; std::string pxdClustersName = "";

    if (mcRecoTracks.getEntries() > 0) {
      svdClustersName = mcRecoTracks[0]->getStoreArrayNameOfSVDHits();
      pxdClustersName = mcRecoTracks[0]->getStoreArrayNameOfPXDHits();
    } else {
      B2WARNING("No Entries in mcRecoTracksStoreArray: using empty cluster name for svd and pxd");
    }

    QualityEstimatorMC* MCestimator = static_cast<QualityEstimatorMC*>(m_estimator.get());
    MCestimator->setClustersNames(svdClustersName, pxdClustersName);
  }
}

void TrackCandidateResultRefiner::apply(std::vector<SpacePointTrackCand>& unprunedResults,
                                        std::vector<SpacePointTrackCand>& prunedResults)
{
  // assign a QI computed using the selected QualityEstimator for each given SpacePointTrackCand
  for (SpacePointTrackCand& aTrackCandidate : unprunedResults) {
    double qi = m_estimator->estimateQuality(aTrackCandidate.getSortedHits());
    aTrackCandidate.setQualityIndicator(qi);
  }

  std::sort(unprunedResults.begin(), unprunedResults.end(),
  [](const SpacePointTrackCand & a, const SpacePointTrackCand & b) {
    return a.getQualityIndicator() < b.getQualityIndicator();
  });

  std::swap(unprunedResults, prunedResults);
}

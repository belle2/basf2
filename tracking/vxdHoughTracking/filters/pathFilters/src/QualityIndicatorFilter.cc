/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/vxdHoughTracking/filters/pathFilters/QualityIndicatorFilter.h>
#include <tracking/trackFindingCDC/filters/base/Filter.icc.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorCircleFit.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorMC.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorRiemannHelixFit.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorTripletFit.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <framework/core/ModuleParamList.templateDetails.h>
#include <framework/geometry/BFieldManager.h>

using namespace Belle2;
using namespace TrackFindingCDC;
using namespace vxdHoughTracking;

void QualityIndicatorFilter::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "trackQualityEstimationMethod"), m_param_EstimationMethod,
                                "Identifier which estimation method to use. Valid identifiers are: [mcInfo, circleFit, tripletFit, helixFit]",
                                m_param_EstimationMethod);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "MCRecoTracksStoreArrayName"), m_param_MCRecoTracksStoreArrayName,
                                "Only required for MCInfo method. Name of StoreArray containing MCRecoTracks.",
                                m_param_MCRecoTracksStoreArrayName);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "MCStrictQualityEstimator"), m_param_MCStrictQualityEstimator,
                                "Only required for MCInfo method. If false combining several MCTracks is allowed.",
                                m_param_MCStrictQualityEstimator);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "QICut"), m_param_QIcut,
                                "Cut on the quality indicator. Only process QI values larger than this.", m_param_QIcut);
}

void QualityIndicatorFilter::beginRun()
{
  const double bFieldZ = BFieldManager::getField(0, 0, 0).Z() / Unit::T;
  m_estimator->setMagneticFieldStrength(bFieldZ);

  if (m_param_EstimationMethod == "mcInfo") {
    QualityEstimatorMC* MCestimator = static_cast<QualityEstimatorMC*>(m_estimator.get());
    MCestimator->forceUpdateClusterNames();
  }
}

void QualityIndicatorFilter::initialize()
{
  // create pointer to chosen estimator
  if (m_param_EstimationMethod == "mcInfo") {
    StoreArray<RecoTrack> mcRecoTracks;
    mcRecoTracks.isRequired(m_param_MCRecoTracksStoreArrayName);
    m_estimator = std::make_unique<QualityEstimatorMC>(m_param_MCRecoTracksStoreArrayName, m_param_MCStrictQualityEstimator);
  } else if (m_param_EstimationMethod == "tripletFit") {
    m_estimator = std::make_unique<QualityEstimatorTripletFit>();
  } else if (m_param_EstimationMethod == "circleFit") {
    m_estimator = std::make_unique<QualityEstimatorCircleFit>();
  } else if (m_param_EstimationMethod == "helixFit") {
    m_estimator = std::make_unique<QualityEstimatorRiemannHelixFit>();
  }
  B2ASSERT("QualityEstimator could not be initialized with method: " << m_param_EstimationMethod, m_estimator);
}

TrackFindingCDC::Weight
QualityIndicatorFilter::operator()(const BasePathFilter::Object& pair)
{
  const std::vector<TrackFindingCDC::WithWeight<const VXDHoughState*>>& previousHits = pair.first;

  std::vector<const SpacePoint*> spacePoints;
  spacePoints.reserve(previousHits.size() + 1);
  for (auto& hit : previousHits) {
    spacePoints.emplace_back(hit->getHit());
  }
  spacePoints.emplace_back(pair.second->getHit());

  // The path is outwards-in, and thus the SPs are added outwards-in, too.
  // The tripletFit only works with hits inside-out, so reverse the SP vector
  std::reverse(spacePoints.begin(), spacePoints.end());

  const auto& estimatorResult = m_estimator->estimateQualityAndProperties(spacePoints);

  if (estimatorResult.qualityIndicator < m_param_QIcut) {
    return NAN;
  }

  return estimatorResult.qualityIndicator;
}

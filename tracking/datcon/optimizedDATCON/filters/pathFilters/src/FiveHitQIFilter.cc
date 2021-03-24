/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Wessel                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/datcon/optimizedDATCON/filters/pathFilters/FiveHitQIFilter.h>
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

void FiveHitQIFilter::beginRun()
{
  const double bFieldZ = BFieldManager::getField(0, 0, 0).Z() / Unit::T;
  m_estimator->setMagneticFieldStrength(bFieldZ);
}

void FiveHitQIFilter::initialize()
{
  // create pointer to chosen estimator
//  if (m_EstimationMethod == "mcInfo") {
//    m_estimator = std::make_unique<QualityEstimatorMC>(m_MCRecoTracksStoreArrayName, m_MCStrictQualityEstimator);
//  } else
  if (m_EstimationMethod == "tripletFit") {
    m_estimator = std::make_unique<QualityEstimatorTripletFit>();
  } else if (m_EstimationMethod == "circleFit") {
    m_estimator = std::make_unique<QualityEstimatorCircleFit>();
  } else if (m_EstimationMethod == "helixFit") {
    m_estimator = std::make_unique<QualityEstimatorRiemannHelixFit>();
  }
  B2ASSERT("QualityEstimator could not be initialized with method: " << m_EstimationMethod, m_estimator);
}

TrackFindingCDC::Weight
FiveHitQIFilter::operator()(const BasePathFilter::Object& pair)
{
  const std::vector<TrackFindingCDC::WithWeight<const HitData*>>& previousHits = pair.first;

  // Do nothing if path is too short or too long
  if (previousHits.size() != 4) {
    return NAN;
  }

  std::vector<const SpacePoint*> spacePoints;
  spacePoints.reserve(previousHits.size() + 1);
  spacePoints.emplace_back(previousHits.at(0)->getHit());
  spacePoints.emplace_back(previousHits.at(1)->getHit());
  spacePoints.emplace_back(previousHits.at(2)->getHit());
  spacePoints.emplace_back(previousHits.at(3)->getHit());
  spacePoints.emplace_back(pair.second->getHit());
  const auto& estimatorResult = m_estimator->estimateQualityAndProperties(spacePoints);

//   const double absHelixPocaD = (estimatorResult.pocaD) ? fabs(*estimatorResult.pocaD) : 1e-6;
//   const double chi2 = (estimatorResult.chiSquared) ? *estimatorResult.chiSquared : 1e6;
//
//   if (absHelixPocaD > m_helixFitPocaDCut) {
//     return NAN;
//   }

  return estimatorResult.qualityIndicator;
}

void FiveHitQIFilter::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "helixFitPocaDCut"), m_helixFitPocaDCut,
                                "Cut on the POCA difference in xy with the POCA obtained from fit with the quality estimator defined by trackQualityEstimationMethod.",
                                m_helixFitPocaDCut);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "trackQualityEstimationMethod"), m_EstimationMethod,
                                "Identifier which estimation method to use. Valid identifiers are: [mcInfo, circleFit, tripletFit, helixFit]",
                                m_EstimationMethod);
//  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "MCRecoTracksStoreArrayName"), m_MCRecoTracksStoreArrayName,
//                               "Only required for MCInfo method. Name of StoreArray containing MCRecoTracks.",
//                                m_MCRecoTracksStoreArrayName);
//  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "MCStrictQualityEstimator"), m_MCStrictQualityEstimator,
//                                "Only required for MCInfo method. If false combining several MCTracks is allowed.",
//                                m_MCStrictQualityEstimator);
}

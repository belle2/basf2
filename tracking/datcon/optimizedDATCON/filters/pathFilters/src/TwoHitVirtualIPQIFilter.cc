/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Wessel                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/datcon/optimizedDATCON/filters/pathFilters/TwoHitVirtualIPQIFilter.h>
#include <tracking/trackFindingCDC/filters/base/Filter.icc.h>

#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorMC.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorRiemannHelixFit.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorTripletFit.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <framework/core/ModuleParamList.templateDetails.h>
#include <framework/geometry/BFieldManager.h>

using namespace Belle2;
using namespace TrackFindingCDC;

void TwoHitVirtualIPQIFilter::beginRun()
{
  const double bFieldZ = BFieldManager::getField(0, 0, 0).Z() / Unit::T;
  m_estimator->setMagneticFieldStrength(bFieldZ);
}

void TwoHitVirtualIPQIFilter::initialize()
{
  // create pointer to chosen estimator
//  if (m_EstimationMethod == "mcInfo") {
//    m_estimator = std::make_unique<QualityEstimatorMC>(m_MCRecoTracksStoreArrayName, m_MCStrictQualityEstimator);
//  } else
  if (m_EstimationMethod == "tripletFit") {
    m_estimator = std::make_unique<QualityEstimatorTripletFit>();
  }  else if (m_EstimationMethod == "helixFit") {
    m_estimator = std::make_unique<QualityEstimatorRiemannHelixFit>();
  }
  B2ASSERT("QualityEstimator could not be initialized with method: " << m_EstimationMethod, m_estimator);
}

TrackFindingCDC::Weight
TwoHitVirtualIPQIFilter::operator()(const BasePathFilter::Object& pair)
{
  const std::vector<TrackFindingCDC::WithWeight<const HitData*>>& previousHits = pair.first;

  // Do nothing if path is too short or too long
  if (previousHits.size() != 1) {
    return NAN;
  }

  std::vector<const SpacePoint*> spacePointsVirtIP;
  spacePointsVirtIP.reserve(previousHits.size() + 2);
  spacePointsVirtIP.emplace_back(&m_virtualIPSpacePoint);
  spacePointsVirtIP.emplace_back(previousHits.at(0)->getHit());
  spacePointsVirtIP.emplace_back(pair.second->getHit());
  const auto& estimatorResultVirtIP = m_estimator->estimateQualityAndProperties(spacePointsVirtIP);

//   const double absHelixPocaDVirtIP = (estimatorResultVirtIP.pocaD) ? fabs(*estimatorResultVirtIP.pocaD) : 1e-6;
//   const double chi2VirtIP = (estimatorResultVirtIP.chiSquared) ? *estimatorResultVirtIP.chiSquared : 1e6;
//
//   if (absHelixPocaDVirtIP > m_helixFitPocaVirtIPDCut) {
//     return NAN;
//   }

  return estimatorResultVirtIP.qualityIndicator;
}

void TwoHitVirtualIPQIFilter::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "helixFitPocaVirtIPDCut"), m_helixFitPocaVirtIPDCut,
                                "Cut on the POCA difference in xy with the POCA obtained from a helix fit, adding a virtual IP at the origin "
                                "(tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorTripletFit).",
                                m_helixFitPocaVirtIPDCut);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "trackQualityEstimationMethod"), m_EstimationMethod,
                                "Identifier which estimation method to use. Valid identifiers are: [mcInfo, tripletFit, helixFit]",
                                m_EstimationMethod);
//  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "MCRecoTracksStoreArrayName"), m_MCRecoTracksStoreArrayName,
//                               "Only required for MCInfo method. Name of StoreArray containing MCRecoTracks.",
//                                m_MCRecoTracksStoreArrayName);
//  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "MCStrictQualityEstimator"), m_MCStrictQualityEstimator,
//                                "Only required for MCInfo method. If false combining several MCTracks is allowed.",
//                                m_MCStrictQualityEstimator);
}

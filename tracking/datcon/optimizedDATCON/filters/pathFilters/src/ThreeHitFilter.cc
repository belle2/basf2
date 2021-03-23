/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Wessel                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/datcon/optimizedDATCON/filters/pathFilters/ThreeHitFilter.h>
#include <tracking/datcon/optimizedDATCON/filters/pathFilters/ThreeHitVariables.h>
#include <tracking/trackFindingCDC/filters/base/Filter.icc.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <framework/core/ModuleParamList.templateDetails.h>
#include <framework/geometry/BFieldManager.h>

using namespace Belle2;
using namespace TrackFindingCDC;

TrackFindingCDC::Weight
ThreeHitFilter::operator()(const BasePathFilter::Object& pair)
{
  const std::vector<TrackFindingCDC::WithWeight<const HitData*>>& previousHits = pair.first;

  // Do nothing if path is too short or too long
  if (previousHits.size() != 2) {
    return NAN;
  }

  const double bFieldZ = BFieldManager::getField(0, 0, 0).Z() / Unit::T;
  helixFitEstimator.setMagneticFieldStrength(bFieldZ);

  const B2Vector3D& firstHitPos   = previousHits.at(0)->getHit()->getPosition();
  const B2Vector3D& secondHitPos  = previousHits.at(1)->getHit()->getPosition();
  const B2Vector3D& currentHitPos = pair.second->getHit()->getPosition();

  ThreeHitVariables threeHitVariables(firstHitPos, secondHitPos, currentHitPos);
  threeHitVariables.setBFieldZ(bFieldZ);

  if (threeHitVariables.getCosAngleRZSimple() < m_cosRZCut) {
    return NAN;
  }

  const double circleDistanceIP = threeHitVariables.getCircleDistanceIP();

  if (circleDistanceIP > m_circleIPDistanceCut) {
    return NAN;
  }

  std::vector<const SpacePoint*> spacePoints;
  spacePoints.reserve(previousHits.size() + 1);
  spacePoints.emplace_back(previousHits.at(0)->getHit());
  spacePoints.emplace_back(previousHits.at(1)->getHit());
  spacePoints.emplace_back(pair.second->getHit());
  const auto& estimatorResult = helixFitEstimator.estimateQualityAndProperties(spacePoints);

  const double absHelixPocaD = (estimatorResult.pocaD) ? fabs(*estimatorResult.pocaD) : 1e-6;
  const double chi2 = (estimatorResult.chiSquared) ? *estimatorResult.chiSquared : 1e6;

//   B2INFO("PocaD: " << absHelixPocaD << " chi2: " << chi2);

  if (absHelixPocaD > m_helixFitPocaDCut) {
    return NAN;
  }

//   std::vector<const SpacePoint*> spacePointsVirtIP;
//   spacePointsVirtIP.reserve(previousHits.size() + 2);
//   SpacePoint virtualIPSpacePoint = SpacePoint(B2Vector3D(0., 0., 0.), B2Vector3D(0.1, 0.1, 0.5), {0.5, 0.5}, {false, false}, VxdID(0), Belle2::VXD::SensorInfoBase::VXD);
//   spacePointsVirtIP.emplace_back(&virtualIPSpacePoint);
//   spacePointsVirtIP.emplace_back(previousHits.at(0)->getHit());
//   spacePointsVirtIP.emplace_back(previousHits.at(1)->getHit());
//   spacePointsVirtIP.emplace_back(pair.second->getHit());
//   const auto& estimatorResultVirtIP = helixFitEstimator.estimateQualityAndProperties(spacePointsVirtIP);
//
//   const double absHelixPocaDVirtIP = (estimatorResultVirtIP.pocaD) ? fabs(*estimatorResultVirtIP.pocaD) : 1e-6;
//   const double chi2VirtIP = (estimatorResultVirtIP.chiSquared) ? *estimatorResultVirtIP.chiSquared : 1e6;
//
//   B2INFO("Virtual IP: absHelixPocaDVirtIP: " << absHelixPocaDVirtIP << " chi2VirtIP: " << chi2VirtIP);

//   return 1.0 / circleDistanceIP;
//   return 1.0 / absHelixPocaD;
//   return 1.0 / chi2;
  return estimatorResult.qualityIndicator;
}

void ThreeHitFilter::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "cosRZCut"), m_cosRZCut,
                                "Cut on the absolute value of cosine between the vectors (oHit - cHit) and (cHit - iHit).",
                                m_cosRZCut);
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "circleIPDistanceCut"), m_circleIPDistanceCut,
                                "Cut on the difference between circle radius and circle center to check whether the circle is compatible with passing through the IP.",
                                m_circleIPDistanceCut);
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "helixFitPocaDCut"), m_helixFitPocaDCut,
                                "Cut on the POCA difference in xy with the POCA obtained from a helix fit (tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorTripletFit).",
                                m_helixFitPocaDCut);
}

/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Wessel                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/datcon/optimizedDATCON/filters/pathFilters/TwoHitVirtualIPFilter.h>
#include <tracking/datcon/optimizedDATCON/filters/pathFilters/FourHitVariables.h>
#include <tracking/trackFindingCDC/filters/base/Filter.icc.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <framework/core/ModuleParamList.templateDetails.h>
#include <framework/geometry/BFieldManager.h>

using namespace Belle2;
using namespace TrackFindingCDC;

TrackFindingCDC::Weight
TwoHitVirtualIPFilter::operator()(const BasePathFilter::Object& pair)
{
  const std::vector<TrackFindingCDC::WithWeight<const HitData*>>& previousHits = pair.first;

  // Do nothing if path is too short or too long
  if (previousHits.size() != 1) {
    return NAN;
  }

  const double bFieldZ = BFieldManager::getField(0, 0, 0).Z() / Unit::T;
  helixFitEstimator.setMagneticFieldStrength(bFieldZ);
//   const B2Vector3D  virtualIP     = B2Vector3D(0., 0., 0.);
//   const B2Vector3D& lastHitPos    = previousHits.at(0)->getHit()->getPosition();
//   const B2Vector3D& currentHitPos = pair.second->getHit()->getPosition();
//
//   // filter expects hits from outer to inner
//   ThreeHitVariables threeHitVariables(lastHitPos, currentHitPos, virtualIP);
//
//   if (threeHitVariables.getCosAngleRZSimple() < m_cosRZCut) {
//     return NAN;
//   }
//
//   const double circleDistanceIP = threeHitVariables.getCircleDistanceIP();
//
//   if (circleDistanceIP > m_circleIPDistanceCut) {
//     return NAN;
//   }

  std::vector<const SpacePoint*> spacePointsVirtIP;
  spacePointsVirtIP.reserve(previousHits.size() + 2);
  SpacePoint virtualIPSpacePoint = SpacePoint(B2Vector3D(0., 0., 0.), B2Vector3D(0.1, 0.1, 0.5), {0.5, 0.5}, {false, false}, VxdID(0),
                                              Belle2::VXD::SensorInfoBase::VXD);
  spacePointsVirtIP.emplace_back(&virtualIPSpacePoint);
  spacePointsVirtIP.emplace_back(previousHits.at(0)->getHit());
  spacePointsVirtIP.emplace_back(pair.second->getHit());
  const auto& estimatorResultVirtIP = helixFitEstimator.estimateQualityAndProperties(spacePointsVirtIP);

  const double absHelixPocaDVirtIP = (estimatorResultVirtIP.pocaD) ? fabs(*estimatorResultVirtIP.pocaD) : 1e-6;
  const double chi2VirtIP = (estimatorResultVirtIP.chiSquared) ? *estimatorResultVirtIP.chiSquared : 1e6;

  if (absHelixPocaDVirtIP > m_helixFitPocaVirtIPDCut) {
    return NAN;
  }

//   return 1.0 / chi2VirtIP;
  return estimatorResultVirtIP.qualityIndicator;
}

void TwoHitVirtualIPFilter::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "helixFitPocaVirtIPDCut"), m_helixFitPocaVirtIPDCut,
                                "Cut on the POCA difference in xy with the POCA obtained from a helix fit, adding a virtual IP at the origin "
                                "(tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorTripletFit).",
                                m_helixFitPocaVirtIPDCut);
}

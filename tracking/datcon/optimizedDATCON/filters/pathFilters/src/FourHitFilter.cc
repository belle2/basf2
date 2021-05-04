/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Wessel                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/datcon/optimizedDATCON/filters/pathFilters/FourHitFilter.h>
#include <tracking/trackFindingCDC/filters/base/Filter.icc.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <framework/core/ModuleParamList.templateDetails.h>
#include <framework/geometry/BFieldManager.h>

using namespace Belle2;
using namespace TrackFindingCDC;

void FourHitFilter::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "circleRadiusDifferenceCut"), m_param_CircleRadiusDifferenceCut,
                                "TODO: Cut on the absolute value of cosine between the vectors (oHit - cHit) and (cHit - iHit).",
                                m_param_CircleRadiusDifferenceCut);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "circleCenterPositionDifferenceCut"),
                                m_param_CircleCenterPositionDifferenceCut,
                                "TODO: Cut on the difference between circle radius and circle center to check whether the circle is compatible with passing through the IP.",
                                m_param_CircleCenterPositionDifferenceCut);
}

void FourHitFilter::beginRun()
{
  const double bFieldZ = BFieldManager::getField(0, 0, 0).Z() / Unit::T;
  m_fourHitVariables.setBFieldZ(bFieldZ);
}

TrackFindingCDC::Weight
FourHitFilter::operator()(const BasePathFilter::Object& pair)
{
  const std::vector<TrackFindingCDC::WithWeight<const HitData*>>& previousHits = pair.first;

  // Do nothing if path is too short or too long
  if (previousHits.size() != 3) {
    return NAN;
  }

  const B2Vector3D& firstHitPos   = previousHits.at(0)->getHit()->getPosition();
  const B2Vector3D& secondHitPos  = previousHits.at(1)->getHit()->getPosition();
  const B2Vector3D& thirdHitPos   = previousHits.at(2)->getHit()->getPosition();
  const B2Vector3D& currentHitPos = pair.second->getHit()->getPosition();

  m_fourHitVariables.setHits(firstHitPos, secondHitPos, thirdHitPos, currentHitPos);

  if (m_fourHitVariables.getCircleRadiusDifference() > m_param_CircleRadiusDifferenceCut) {
    return NAN;
  }
  if (m_fourHitVariables.getCircleCenterPositionDifference() > m_param_CircleCenterPositionDifferenceCut) {
    return NAN;
  }

  return fabs(1.0 / m_fourHitVariables.getCircleRadiusDifference());
}

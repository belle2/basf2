/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/vxdHoughTracking/filters/pathFilters/FourHitFilter.h>
#include <tracking/trackFindingCDC/filters/base/Filter.icc.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <framework/core/ModuleParamList.templateDetails.h>
#include <framework/geometry/BFieldManager.h>

using namespace Belle2;
using namespace TrackFindingCDC;
using namespace vxdHoughTracking;

void FourHitFilter::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "circleRadiusDifferenceCut"), m_CircleRadiusDifferenceCut,
                                "Cut on the difference of the radii of the two circles that can be defined by two hit triplets.",
                                m_CircleRadiusDifferenceCut);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "circleCenterPositionDifferenceCut"),
                                m_CircleCenterPositionDifferenceCut,
                                "Cut on the difference between the center positions of the two circles that can be defined by two hit triplets.",
                                m_CircleCenterPositionDifferenceCut);
}

void FourHitFilter::beginRun()
{
  const double bFieldZ = BFieldManager::getField(0, 0, 0).Z() / Unit::T;
  m_fourHitVariables.setBFieldZ(bFieldZ);
}

TrackFindingCDC::Weight
FourHitFilter::operator()(const BasePathFilter::Object& pair)
{
  const std::vector<TrackFindingCDC::WithWeight<const VXDHoughState*>>& previousHits = pair.first;

  // Do nothing if path is too short or too long
  if (previousHits.size() != 3) {
    return NAN;
  }

  const B2Vector3D& firstHitPos   = previousHits.at(0)->getHit()->getPosition();
  const B2Vector3D& secondHitPos  = previousHits.at(1)->getHit()->getPosition();
  const B2Vector3D& thirdHitPos   = previousHits.at(2)->getHit()->getPosition();
  const B2Vector3D& currentHitPos = pair.second->getHit()->getPosition();

  m_fourHitVariables.setHits(firstHitPos, secondHitPos, thirdHitPos, currentHitPos);

  if (m_fourHitVariables.getCircleRadiusDifference() > m_CircleRadiusDifferenceCut) {
    return NAN;
  }
  if (m_fourHitVariables.getCircleCenterPositionDifference() > m_CircleCenterPositionDifferenceCut) {
    return NAN;
  }

  return fabs(1.0 / m_fourHitVariables.getCircleRadiusDifference());
}

/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/svd/filters/relations/SensorSVDPairFilter.h>
#include <tracking/trackFindingCDC/filters/base/Filter.icc.h>

#include <tracking/trackFindingCDC/geometry/Vector2D.h>
#include <tracking/trackFindingCDC/geometry/Vector3D.h>

#include <tracking/spacePointCreation/SpacePoint.h>
#include <vxd/geometry/GeoCache.h>

using namespace Belle2;
using namespace TrackFindingCDC;

TrackFindingCDC::Weight
SensorSVDPairFilter::operator()(const std::pair<const CKFToSVDState*, const CKFToSVDState*>& relation)
{
  const CKFToSVDState& fromState = *(relation.first);
  const CKFToSVDState& toState = *(relation.second);

  const CKFToSVDState::stateCache& fromStateCache = fromState.getStateCache();
  const CKFToSVDState::stateCache& toStateCache = toState.getStateCache();

  B2ASSERT("You have filled the wrong states into this!", toStateCache.isHitState);

  if (not fromStateCache.isHitState) {
    // We are coming from a CDC track, so we can use its position to only look for matching ladders
    // TODO: implement a better way, e.g. using
    // const RecoTrack* seed = fromState.getSeed();
    // const auto& cdcPosition = fromState.getMSoPPosition();
    return 1.0;
  }

  if (fromStateCache.geoLayer == toStateCache.geoLayer) {
    // TODO: Also check for sensors?
    return 1.0;
  }

  // Next layer is not an overlap one, so we can just return all hits of the next layer(s)
  // that are close enough in phi. No cut in theta here since this is the SensorSVXDPairFilter,
  // a cut in theta is used in the DistanceSVDPairFilter
  const int sensorNumberDifference =
    static_cast<int>(fromStateCache.sensorID.getSensorNumber()) - static_cast<int>(toStateCache.sensorID.getSensorNumber());
  const int layerNumberDifference =
    static_cast<int>(fromStateCache.geoLayer) - static_cast<int>(toStateCache.geoLayer);

  if ((abs(sensorNumberDifference) > 1 and layerNumberDifference == 1) or (abs(sensorNumberDifference) > 2)) {
    return NAN;
  }
  double angle = fromStateCache.sensorCenterPhi - toStateCache.sensorCenterPhi;
  if (angle > M_PI) angle -= 2. * M_PI;
  if (angle < -M_PI) angle += 2. * M_PI;

  if (fabs(angle) < (M_PI - 2.)) {
    return 1.0;
  }

  return NAN;
}

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
    // This is done with the sensorCenterPhi, using the state's phi (and/or theta) wouldn't be a SensorFilter anymore.
    double phiDiff = fromStateCache.phi - toStateCache.sensorCenterPhi;
    while (phiDiff > M_PI) phiDiff -= 2. * M_PI;
    while (phiDiff < -M_PI) phiDiff += 2. * M_PI;

    if (fabs(phiDiff) < 0.5) {
      return 1.0;
    }
    // If the current state (fromState) is a RecoTrack-based state, but no relations could be created
    // don't proceed but return
    return NAN;
  }

  // On same layer we already know from LayerSVDRelationFilter, that we only deal with overlaps in r-phi.
  // So it's sufficient here to check for same layer number to accept states in the overlap region.
  if (fromStateCache.geoLayer == toStateCache.geoLayer and
      fromStateCache.sensorID.getSensorNumber() == toStateCache.sensorID.getSensorNumber()) {
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

  double phiDiff = fromStateCache.sensorCenterPhi - toStateCache.sensorCenterPhi;
  while (phiDiff > M_PI) phiDiff -= 2. * M_PI;
  while (phiDiff < -M_PI) phiDiff += 2. * M_PI;

  if (fabs(phiDiff) < (M_PI - 2.)) {
    return 1.0;
  }

  return NAN;
}

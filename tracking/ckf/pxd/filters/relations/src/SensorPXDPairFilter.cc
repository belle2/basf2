/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/pxd/filters/relations/SensorPXDPairFilter.h>
#include <tracking/trackFindingCDC/filters/base/Filter.icc.h>

#include <tracking/trackFindingCDC/geometry/Vector2D.h>
#include <tracking/trackFindingCDC/geometry/Vector3D.h>

#include <tracking/spacePointCreation/SpacePoint.h>
#include <vxd/geometry/GeoCache.h>

using namespace Belle2;
using namespace TrackFindingCDC;

TrackFindingCDC::Weight
SensorPXDPairFilter::operator()(const std::pair<const CKFToPXDState*, const CKFToPXDState*>& relation)
{
  const CKFToPXDState& fromState = *(relation.first);
  const CKFToPXDState& toState = *(relation.second);

  const CKFToPXDState::stateCache& fromStateCache = fromState.getStateCache();
  const CKFToPXDState::stateCache& toStateCache = toState.getStateCache();

  B2ASSERT("You have filled the wrong states into this!", toStateCache.isHitState);

  if (not fromStateCache.isHitState) {
    // We are coming from a SVD-CDC track, so we can use its position to only look for matching ladders
    // This is done with the sensorCenterPhi, using the state's phi (and/or theta) wouldn't be a SensorFilter anymore.
    double phiDiff = fromStateCache.phi - toStateCache.sensorCenterPhi;
    while (phiDiff > M_PI) phiDiff -= 2. * M_PI;
    while (phiDiff < -M_PI) phiDiff += 2. * M_PI;

    if (fabs(phiDiff) < 0.3) {
      return 1.0;
    }
    // If the current state (fromState) is a RecoTrack-based state, but no relations could be created
    // don't proceed but return
    return NAN;
  }

  // On same layer we already know from LayerPXDRelationFilter, that we only deal with overlaps in r-phi.
  // So it's sufficient here to check for same layer number to accept states in the overlap region.
  if (fromStateCache.geoLayer == toStateCache.geoLayer and
      fromStateCache.sensorID.getSensorNumber() == toStateCache.sensorID.getSensorNumber()) {
    // TODO: Checking for equality of sensor numbers seems not to harm the hit efficiency,
    // but maybe it's safer to allow for a sensor number difference of 1?
    return 1.0;
  }

  // Next layer is not an overlap one, so we can just return all hits of the next layer
  // that are close enough in phi. No cut in theta here since this is the SensorPXDPairFilter,
  // a cut in theta is used in the DistancePXDPairFilter
  const int sensorNumberDifference =
    static_cast<int>(fromStateCache.sensorID.getSensorNumber()) - static_cast<int>(toStateCache.sensorID.getSensorNumber());
  const int layerNumberDifference =
    static_cast<int>(fromStateCache.geoLayer) - static_cast<int>(toStateCache.geoLayer);

  if ((abs(sensorNumberDifference) > 1 and layerNumberDifference == 1) or (abs(sensorNumberDifference) > 2)) {
    return false;
  }

  double phiDiff = fromStateCache.sensorCenterPhi - toStateCache.sensorCenterPhi;
  while (phiDiff > M_PI) phiDiff -= 2. * M_PI;
  while (phiDiff < -M_PI) phiDiff += 2. * M_PI;

  if (fabs(phiDiff) < (M_PI - 2.)) {
    return 1.0;
  }

  return NAN;
}

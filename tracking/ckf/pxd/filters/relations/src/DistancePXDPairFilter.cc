/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun, Christian Wessel                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/pxd/filters/relations/DistancePXDPairFilter.h>
#include <tracking/trackFindingCDC/filters/base/Filter.icc.h>

#include <tracking/spacePointCreation/SpacePoint.h>

using namespace Belle2;
using namespace TrackFindingCDC;

TrackFindingCDC::Weight
DistancePXDPairFilter::operator()(const std::pair<const CKFToPXDState*, const CKFToPXDState*>& relation)
{
  const CKFToPXDState& fromState = *(relation.first);
  const CKFToPXDState& toState = *(relation.second);

  const CKFToPXDState::stateCache& fromStateCache = fromState.getStateCache();
  const CKFToPXDState::stateCache& toStateCache = toState.getStateCache();

  B2ASSERT("You have filled the wrong states into this!", toStateCache.isHitState);

  float phiDiff = fromStateCache.phi - toStateCache.phi;
  while (phiDiff > M_PI) phiDiff -= 2. * M_PI;
  while (phiDiff < -M_PI) phiDiff += 2. * M_PI;

  if (not fromStateCache.isHitState) {
    // We are coming from an SVD track, so we can use its position to only look for matching ladders
    if (abs(phiDiff) < 0.2f) {
      return 1.0;
    }

    return NAN;
  }

  if (fromStateCache.geoLayer == toStateCache.geoLayer and
      fromStateCache.sensorID.getSensorNumber() == toStateCache.sensorID.getSensorNumber()) {
    // TODO: Checking for equality of sensor numbers seems not to harm the hit efficiency,
    // but maybe it's safer to allow for a sensor number difference of 1?
    return 1.0;
  }

  if (abs(phiDiff) < 0.05f) {
    return 1.0;
  }

  return NAN;
}

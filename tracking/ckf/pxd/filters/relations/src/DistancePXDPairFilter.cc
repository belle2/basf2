/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
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

  const double& toPhi = toStateCache.phi;

  if (not fromStateCache.isHitState) {
    // We are coming from an SVD track, so we can use its position to only look for matching ladders
    const genfit::MeasuredStateOnPlane& mSoP = fromState.getMeasuredStateOnPlane();
    const double fromPhi = mSoP.getPos().Phi();

    if (abs(fromPhi - toPhi) < 0.2) {
      return 1.0;
    }

    return NAN;
  }

  if (fromStateCache.geoLayer == toStateCache.geoLayer) {
    // TODO: Also check for sensors?
    return 1.0;
  }

  const double& fromPhi = fromStateCache.phi;

  if (abs(fromPhi - toPhi) < 0.05) {
    return 1.0;
  }

  return NAN;
}

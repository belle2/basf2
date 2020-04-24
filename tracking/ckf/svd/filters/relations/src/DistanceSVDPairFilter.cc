/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/svd/filters/relations/DistanceSVDPairFilter.h>
#include <tracking/trackFindingCDC/filters/base/Filter.icc.h>

#include <tracking/spacePointCreation/SpacePoint.h>

using namespace Belle2;
using namespace TrackFindingCDC;

TrackFindingCDC::Weight
DistanceSVDPairFilter::operator()(const std::pair<const CKFToSVDState*, const CKFToSVDState*>& relation)
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

  const double& fromPhi = fromStateCache.phi;
  const double& toPhi = toStateCache.phi;

  if (abs(fromPhi - toPhi) < 0.2) {
    return 1.0;
  }

  return NAN;
}

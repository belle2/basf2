/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/cdc/filters/states/MCTruthCDCStateFilter.h>

#include <tracking/ckf/cdc/entities/CDCCKFState.h>

using namespace Belle2;

TrackFindingCDC::Weight MCTruthCDCStateFilter::operator()(const BaseCDCStateFilter::Object& pair)
{
  const auto& path = *(pair.first);
  const auto& state = *(pair.second);

  const auto& seed = path.front();
  const auto* seedMCRecoTrack = seed.getMCRecoTrack();

  if (not seedMCRecoTrack) {
    return NAN;
  }

  const auto* wireHit = state.getWireHit();
  const auto* cdcHit = wireHit->getHit();

  const auto* hitMCRecoTrack = cdcHit->getRelated<RecoTrack>("MCRecoTracks");

  if (seedMCRecoTrack != hitMCRecoTrack) {
    return NAN;
  }

  return 1;
}

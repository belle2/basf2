/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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

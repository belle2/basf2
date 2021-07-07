/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/ckf/cdc/filters/states/MCTruthEclSeedFilter.h>

#include <tracking/ckf/cdc/entities/CDCCKFState.h>

#include <ecl/dataobjects/ECLShower.h>

#include <mdst/dataobjects/MCParticle.h>

using namespace Belle2;

TrackFindingCDC::Weight MCTruthEclSeedFilter::operator()(const BaseCDCStateFilter::Object& pair)
{
  const auto& path = *(pair.first);
  const auto& state = *(pair.second);

  const auto& seed = path.front();
  const auto* seedRecoTrack = seed.getSeed();
  const auto* seedEclShower = seedRecoTrack->getRelated<ECLShower>("ECLShowers");
  const auto* seedMCParticle = seedEclShower->getRelated<MCParticle>();

  const auto* wireHit = state.getWireHit();
  const auto* cdcHit = wireHit->getHit();
  const auto* hitMCParticle = cdcHit->getRelated<MCParticle>();

  if (seedMCParticle != hitMCParticle) {
    return NAN;
  }

  return 1;
}

/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Simon Kurz, Nils Braun                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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

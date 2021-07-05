/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/ckf/cdc/filters/states/CDCStateTruthVarSet.h>

#include <tracking/ckf/cdc/entities/CDCCKFState.h>

#include <mdst/dataobjects/MCParticle.h>

#include <tracking/dataobjects/RecoTrack.h>

using namespace std;
using namespace Belle2;

bool CDCStateTruthVarSet::extract(const BaseCDCStateFilter::Object* pair)
{
  const auto& path = pair->first;
  const auto& state = pair->second;

  // check if hit belongs to same seed
  const auto& seed = path->front();
  const auto* seedRecoTrack = seed.getSeed();
  const auto* seedMCTrack = seedRecoTrack->getRelated<RecoTrack>("MCRecoTracks");
  const auto* seedMCParticle = seedMCTrack->getRelated<MCParticle>();

  const auto* wireHit = state->getWireHit();
  const auto* cdcHit = wireHit->getHit();
  const auto* hitMCTrack = cdcHit->getRelated<RecoTrack>("MCRecoTracks");

  // Bremsstrahlung etc (works for electron gun, check for other events later)
  while (seedMCParticle->getMother()) {
    seedMCParticle = seedMCParticle->getMother();
  }

  // calculate the interesting quantities
  var<named("match")>() = seedMCTrack == hitMCTrack ? true : false;
  var<named("PDG")>() = seedMCParticle->getPDG();

  return true;
}

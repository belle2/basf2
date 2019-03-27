/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Simon Kurz, Nils Braun                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/cdc/filters/paths/CDCfromEclPathTruthVarSet.h>

#include <tracking/ckf/cdc/entities/CDCCKFState.h>
#include <tracking/ckf/cdc/entities/CDCCKFPath.h>

#include <ecl/dataobjects/ECLShower.h>
#include <mdst/dataobjects/MCParticle.h>

#include <tracking/dataobjects/RecoTrack.h>

using namespace std;
using namespace Belle2;

bool CDCfromEclPathTruthVarSet::extract(const BaseCDCPathFilter::Object* path)
{
  // check if hit belongs to same seed
  const auto& seed = path->front();
  const auto* seedRecoTrack = seed.getSeed();
  const auto* seedEclShower = seedRecoTrack->getRelated<ECLShower>("ECLShowers");
  const auto* seedMCParticle = seedEclShower->getRelated<MCParticle>();
  const auto* seedMCTrack = seedRecoTrack->getRelated<RecoTrack>("MCRecoTracks");

  while (seedMCParticle->getMother()) {
    seedMCParticle = seedMCParticle->getMother();
  }

  int matched = 0;

  for (auto const& state : *path) {
    if (state.isSeed()) {
      continue;
    }

    const auto wireHit = state.getWireHit();
    const auto cdcHit = wireHit->getHit();
    // const auto* hitMCParticle = cdcHit->getRelated<MCParticle>();
    const auto* hitMCTrack = cdcHit->getRelated<RecoTrack>("MCRecoTracks");

    if (seedMCTrack == hitMCTrack) {
      matched += 1;
    }
  }

  var<named("matched")>() = matched;
  var<named("PDG")>() = seedMCParticle->getPDG();

  auto seedMom = seedMCParticle->getMomentum();
  var<named("seed_p_truth")>() = seedMom.Mag();
  var<named("seed_pt_truth")>() = seedMom.Perp();
  var<named("seed_pz_truth")>() = seedMom.Z();

  return true;
}

/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/ckf/cdc/filters/paths/CDCfromEclPathTruthVarSet.h>

#include <tracking/ckf/cdc/entities/CDCCKFState.h>

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

  int daughters = 0;
  std::vector<MCParticle*> daughterMCParticles;
  if (seedMCParticle->getNDaughters() > 0) {
    daughterMCParticles = seedMCParticle->getDaughters();
  }

  int matched = 0;

  for (auto const& state : *path) {
    if (state.isSeed()) {
      continue;
    }

    const auto wireHit = state.getWireHit();
    const auto cdcHit = wireHit->getHit();
    auto* hitMCTrack = cdcHit->getRelated<RecoTrack>("MCRecoTracks");

    if (seedMCTrack != 0 && seedMCTrack == hitMCTrack) {
      matched += 1;
    }
  }


  var<named("matched")>() = matched;
  var<named("daughters")>() = daughters;
  var<named("PDG")>() = seedMCParticle->getPDG();

  auto seedMom = seedMCParticle->getMomentum();
  var<named("seed_p_truth")>() = seedMom.Mag();
  var<named("seed_theta_truth")>() = seedMom.Theta() * 180. / M_PI;
  var<named("seed_pt_truth")>() = seedMom.Perp();
  var<named("seed_pz_truth")>() = seedMom.Z();
  var<named("seed_px_truth")>() = seedMom.X();
  var<named("seed_py_truth")>() = seedMom.Y();

  // before, I used "RecoTracks" instead so I could get the msop.
  // Gets track of mother particle (be careful, eg for photon guns)
  const auto* particleMCTrack = seedMCParticle->getRelated<RecoTrack>("MCRecoTracks");
  int mcTrackHits = -1;
  if (particleMCTrack) {
    mcTrackHits = particleMCTrack->getNumberOfCDCHits();
  }
  var<named("mcTrackHits")>() = mcTrackHits;

  return true;
}

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

  // don't do this for photon gun etc
  //while (seedMCParticle->getMother()) {
  //  seedMCParticle = seedMCParticle->getMother();
  //}

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
    // const auto* hitMCParticle = cdcHit->getRelated<MCParticle>();
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
  //const auto* particleMCTrack = seedMCTrack;
  int mcTrackHits = -1;
  TVector3 trackPos(0, 0, 0);
  TVector3 trackMom(0, 0, 0);
  if (particleMCTrack) {
    mcTrackHits = particleMCTrack->getNumberOfCDCHits();
    //auto msop = particleMCTrack->getMeasuredStateOnPlaneFromLastHit();
    //trackPos = msop.getPos();
    //trackMom = msop.getMom();
  }
  var<named("mcTrackHits")>() = mcTrackHits;

  var<named("mcTrackEnd_p")>() = trackMom.Mag();
  var<named("mcTrackEnd_momTheta")>() = trackMom.Theta() * 180. / M_PI;
  var<named("mcTrackEnd_pt")>() = trackMom.Perp();
  var<named("mcTrackEnd_pz")>() = trackMom.Z();
  var<named("mcTrackEnd_px")>() = trackMom.X();
  var<named("mcTrackEnd_py")>() = trackMom.Y();

  var<named("mcTrackEnd_z")>() = trackPos.Z();
  var<named("mcTrackEnd_x")>() = trackPos.X();
  var<named("mcTrackEnd_y")>() = trackPos.Y();
  var<named("mcTrackEnd_posTheta")>() = trackPos.Theta() * 180. / M_PI;

  return true;
}

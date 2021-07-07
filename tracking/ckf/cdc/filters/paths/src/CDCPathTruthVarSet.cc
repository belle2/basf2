/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/ckf/cdc/filters/paths/CDCPathTruthVarSet.h>

#include <tracking/ckf/cdc/entities/CDCCKFState.h>

#include <ecl/dataobjects/ECLShower.h>
#include <mdst/dataobjects/MCParticle.h>

#include <tracking/dataobjects/RecoTrack.h>

using namespace std;
using namespace Belle2;

bool CDCPathTruthVarSet::extract(const BaseCDCPathFilter::Object* path)
{
  // check if hit belongs to same seed
  const auto& seed = path->front();
  auto* seedRecoTrack = seed.getSeed();
  auto* seedMCTrack = seedRecoTrack->getRelated<RecoTrack>("MCRecoTracks");

  MCParticle* seedMCParticle;
  if (seedMCTrack) {
    seedMCParticle = seedMCTrack->getRelated<MCParticle>();
  }
  // maybe used track from Ecl seeding?
  else {
    seedRecoTrack = seedRecoTrack->getRelated<RecoTrack>("EclSeedRecoTracks");
    seedMCTrack = seedRecoTrack->getRelated<RecoTrack>("MCRecoTracks");
    const auto* seedEclShower = seedRecoTrack->getRelated<ECLShower>("ECLShowers");
    seedMCParticle = seedEclShower->getRelated<MCParticle>();
  }

  int daughters = 0;
  std::vector<MCParticle*> daughterMCParticles;
  if (seedMCParticle->getNDaughters() > 0) {
    daughterMCParticles = seedMCParticle->getDaughters();

    while (daughterMCParticles.size() == 1) {
      daughterMCParticles = daughterMCParticles.at(0)->getDaughters();
    }
    if (daughterMCParticles.size() > 1) {
      daughters = daughterMCParticles.size();
    }
  }

  while (seedMCParticle->getMother()) {
    seedMCParticle = seedMCParticle->getMother();
  }

  int mcTrackHits = 0;

  if (seedMCTrack) {
    mcTrackHits = seedMCTrack->getNumberOfCDCHits();
  }
  var<named("mcTrackHits")>() = mcTrackHits;

  var<named("daughters")>() = daughters;
  var<named("PDG")>() = seedMCParticle->getPDG();

  auto seedMom = seedMCParticle->getMomentum();
  var<named("seed_p_truth")>() = seedMom.Mag();
  var<named("seed_theta_truth")>() = seedMom.Theta() * 180. / M_PI;
  var<named("seed_pt_truth")>() = seedMom.Perp();
  var<named("seed_pz_truth")>() = seedMom.Z();
  var<named("seed_px_truth")>() = seedMom.X();
  var<named("seed_py_truth")>() = seedMom.Y();

  return true;
}

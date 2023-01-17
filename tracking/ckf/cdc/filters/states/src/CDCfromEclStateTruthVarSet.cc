/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/ckf/cdc/filters/states/CDCfromEclStateTruthVarSet.h>

#include <tracking/ckf/cdc/entities/CDCCKFState.h>
#include <ecl/dataobjects/ECLShower.h>

#include <mdst/dataobjects/MCParticle.h>

#include <tracking/dataobjects/RecoTrack.h>

using namespace Belle2;

bool CDCfromEclStateTruthVarSet::extract(const BaseCDCStateFilter::Object* pair)
{
  const auto& path = pair->first;
  const auto& state = pair->second;

  // check if hit belongs to same seed
  const auto& seed = path->front();
  const auto* seedRecoTrack = seed.getSeed();
  const auto* seedEclShower = seedRecoTrack->getRelated<ECLShower>("ECLShowers");
  const auto* seedMCParticle = seedEclShower->getRelated<MCParticle>();

  const auto* wireHit = state->getWireHit();
  const auto* cdcHit = wireHit->getHit();
  const auto* hitMCParticle = cdcHit->getRelated<MCParticle>();

  // Bremsstrahlung etc (works for electron gun, check for other events later)
  while (hitMCParticle->getMother()) {
    hitMCParticle = hitMCParticle->getMother();
  }

  // calculate the interesting quantities
  var<named("match")>() = seedMCParticle == hitMCParticle ? true : false;
  var<named("PDG")>() = seedMCParticle->getPDG();

  auto seedMom = seedMCParticle->getMomentum();
  var<named("seed_p_truth")>() = seedMom.R();
  var<named("seed_theta_truth")>() = seedMom.Theta() * 180. / M_PI;
  var<named("seed_pt_truth")>() = seedMom.Rho();
  var<named("seed_pz_truth")>() = seedMom.Z();
  var<named("seed_px_truth")>() = seedMom.X();
  var<named("seed_py_truth")>() = seedMom.Y();

  return true;
}

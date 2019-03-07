/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Simon Kurz, Nils Braun                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/cdc/filters/states/CDCStateTruthVarSet.h>

#include <tracking/ckf/cdc/entities/CDCCKFState.h>
#include <tracking/ckf/cdc/entities/CDCCKFPath.h>
#include <ecl/dataobjects/ECLShower.h>

#include <ecl/dataobjects/ECLSimHit.h>


#include <mdst/dataobjects/MCParticle.h>

#include <tracking/dataobjects/RecoTrack.h>

using namespace std;
using namespace Belle2;

bool CDCStateTruthVarSet::extract(const BaseCDCStateFilter::Object* pair)
{
  const auto& path = pair->first;
  const auto& state = pair->second;
  const auto& lastState = path->back();

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

  bool match = false;
  if (seedMCParticle == hitMCParticle) {
    match = true;
  }/*else{
    B2INFO("seed:" << seedMCParticle->getPDG());
    if(seedMCParticle->getMother()){
      B2INFO("->" << seedMCParticle->getMother()->getPDG());
      if(seedMCParticle->getMother()->getMother()){
        B2INFO("-->" << seedMCParticle->getMother()->getMother()->getPDG());
      }
    }
    B2INFO("trk:" << hitMCParticle->getPDG());
    if(hitMCParticle->getMother()){
      B2INFO("->" << hitMCParticle->getMother()->getPDG());
      if(hitMCParticle->getMother()->getMother()){
        B2INFO("-->" << hitMCParticle->getMother()->getMother()->getPDG());
      }
    }
  }*/

  //if(lastState.isSeed()){
  //  const auto* seedEclSimHit = seedMCParticle->getRelated<ECLSimHit>("ECLSimHits");
  //  const G4ThreeVector simHitPos = seedEclSimHit->getPosition();
  //  B2INFO("SimHit: " << simHitPos.x()*simHitPos.x() + simHitPos.y()*simHitPos.y() << "," << simHitPos.z());
  //  B2INFO("Seed:   " << seedRecoTrack->getPositionSeed().x()*seedRecoTrack->getPositionSeed().x() + seedRecoTrack->getPositionSeed().y()*seedRecoTrack->getPositionSeed().y() << "," << seedRecoTrack->getPositionSeed().z());
  //}

  // calculate the interesting quantities
  var<named("match")>() = match;
  var<named("PDG")>() = seedMCParticle->getPDG();
  var<named("firstHit")>() = lastState.isSeed();
  var<named("arcLength")>() = state->getArcLength() - lastState.getArcLength();
  var<named("hitDistance")>() = state->getHitDistance();

  TVector3 seedPos = seedRecoTrack->getPositionSeed();
  var<named("seed_r")>() = seedPos.Perp();
  var<named("seed_z")>() = seedPos.Z();


  return true;
}

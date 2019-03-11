/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Simon Kurz, Nils Braun                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/cdc/filters/states/CDCStateBasicVarSet.h>

#include <tracking/ckf/cdc/entities/CDCCKFState.h>
#include <tracking/ckf/cdc/entities/CDCCKFPath.h>


#include <mdst/dataobjects/MCParticle.h>

#include <tracking/dataobjects/RecoTrack.h>

using namespace std;
using namespace Belle2;

bool CDCStateBasicVarSet::extract(const BaseCDCStateFilter::Object* pair)
{
  const auto& path = pair->first;
  const auto& state = pair->second;
  const auto& lastState = path->back();

  // check if hit belongs to same seed
  const auto& seed = path->front();
  const auto* seedRecoTrack = seed.getSeed();

  //const auto* wireHit = state->getWireHit();
  //const auto* cdcHit = wireHit->getHit();

  // calculate the interesting quantities
  var<named("firstHit")>() = lastState.isSeed();
  var<named("arcLength")>() = state->getArcLength() - lastState.getArcLength();
  var<named("hitDistance")>() = state->getHitDistance();
  var<named("iCLayer")>() = state->getWireHit()->getWire().getICLayer();

  TVector3 seedPos = seedRecoTrack->getPositionSeed();
  var<named("seed_r")>() = seedPos.Perp();
  var<named("seed_z")>() = seedPos.Z();
  var<named("seed_charge")>() = seedRecoTrack->getChargeSeed();

  return true;
}

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

  // general stuff
  var<named("eventNumber")>() = m_eventMetaData->getEvent();

  // calculate the interesting quantities
  var<named("firstHit")>() = lastState.isSeed();
  var<named("iCLayer")>() = state->getWireHit()->getWire().getICLayer();
  var<named("arcLength")>() = state->getArcLength() - lastState.getArcLength();
  var<named("hitDistance")>() = state->getHitDistance();

  TVector3 seedPos = seedRecoTrack->getPositionSeed();
  TVector3 seedMom = seedRecoTrack->getMomentumSeed();
  var<named("seed_r")>() = seedPos.Perp();
  var<named("seed_z")>() = seedPos.Z();
  var<named("seed_p")>() = seedMom.Mag();
  var<named("seed_pt")>() = seedMom.Perp();
  var<named("seed_pz")>() = seedMom.Z();
  var<named("seed_charge")>() = seedRecoTrack->getChargeSeed();

  // track representation
  genfit::MeasuredStateOnPlane trackState = lastState.getTrackState();
  //TVector3 trackPosition = trackState.getPos();
  TVector3 trackMom = trackState.getMom();
  var<named("track_p")>() = trackMom.Mag();
  var<named("track_pt")>() = trackMom.Perp();
  var<named("track_pz")>() = trackMom.Z();
  var<named("track_charge")>() = trackState.getCharge();

  return true;
}

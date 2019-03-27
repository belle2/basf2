/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Simon Kurz, Nils Braun                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/cdc/filters/paths/CDCPathBasicVarSet.h>

#include <tracking/ckf/cdc/entities/CDCCKFState.h>
#include <tracking/ckf/cdc/entities/CDCCKFPath.h>


#include <mdst/dataobjects/MCParticle.h>

#include <tracking/dataobjects/RecoTrack.h>

using namespace std;
using namespace Belle2;

bool CDCPathBasicVarSet::extract(const BaseCDCPathFilter::Object* path)
{
  // check if hit belongs to same seed
  const auto& seed = path->front();
  const auto* seedRecoTrack = seed.getSeed();

  double arcLength = 0;
  std::vector<double> arcLengths;
  arcLengths.reserve(path->size() - 1);
  std::vector<double> hitDistances;
  hitDistances.reserve(path->size() - 1);

  int chargeFlip = 0;
  int lastCharge = seedRecoTrack->getChargeSeed();
  std::string str("Full path: ");
  for (auto const& state : *path) {
    if (state.isSeed()) {
      continue;
    }

    arcLength = state.getArcLength() - arcLength;
    arcLengths.push_back(arcLength);

    hitDistances.push_back(state.getHitDistance());


    // check how many times genfit changed the charge
    genfit::MeasuredStateOnPlane trackState = state.getTrackState();
    int stateCharge = trackState.getCharge();
    if (lastCharge != stateCharge) {
      chargeFlip += 1;
      lastCharge = stateCharge;
    }

    // some debugging
    str += std::to_string(state.getWireHit()->getWire().getICLayer());
    str += "(";
    str += std::to_string(state.getHitDistance());
    str += ") - ";
  }
  // B2INFO(str);

  // general stuff
  var<named("eventNumber")>() = m_eventMetaData->getEvent();

  // track properties
  var<named("totalHits")>() = path->size() - 1;
  var<named("chargeFlip")>() = chargeFlip;

  // seed properties
  TVector3 seedPos = seedRecoTrack->getPositionSeed();
  TVector3 seedMom = seedRecoTrack->getMomentumSeed();
  var<named("seed_r")>() = seedPos.Perp();
  var<named("seed_z")>() = seedPos.Z();
  var<named("seed_p")>() = seedMom.Mag();
  var<named("seed_pt")>() = seedMom.Perp();
  var<named("seed_pz")>() = seedMom.Z();
  var<named("seed_charge")>() = seedRecoTrack->getChargeSeed();

  // track representation
  if (path->size() > 1) {
    genfit::MeasuredStateOnPlane trackState = path->back().getTrackState();
    //TVector3 trackPosition = trackState.getPos();
    TVector3 trackMom = trackState.getMom();
    var<named("track_p")>() = trackMom.Mag();
    var<named("track_pt")>() = trackMom.Perp();
    var<named("track_pz")>() = trackMom.Z();
    var<named("track_charge")>() = trackState.getCharge();
  }

  if (path->size() > 10) {
    var<named("hitDistance0")>() = hitDistances[0];
    var<named("hitDistance1")>() = hitDistances[1];
    var<named("hitDistance2")>() = hitDistances[2];
    var<named("hitDistance3")>() = hitDistances[3];
    var<named("hitDistance4")>() = hitDistances[4];
    var<named("hitDistance5")>() = hitDistances[5];
    var<named("hitDistance6")>() = hitDistances[6];
    var<named("hitDistance7")>() = hitDistances[7];
    var<named("hitDistance8")>() = hitDistances[8];
    var<named("hitDistance9")>() = hitDistances[9];
    var<named("arcLength0")>() = arcLengths[0];
    var<named("arcLength1")>() = arcLengths[1];
    var<named("arcLength2")>() = arcLengths[2];
    var<named("arcLength3")>() = arcLengths[3];
    var<named("arcLength4")>() = arcLengths[4];
    var<named("arcLength5")>() = arcLengths[5];
    var<named("arcLength6")>() = arcLengths[6];
    var<named("arcLength7")>() = arcLengths[7];
    var<named("arcLength8")>() = arcLengths[8];
    var<named("arcLength9")>() = arcLengths[9];
  }

  return true;
}

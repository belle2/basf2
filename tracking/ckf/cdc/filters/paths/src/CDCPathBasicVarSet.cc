/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Simon Kurz, Nils Braun                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/cdc/filters/paths/CDCPathBasicVarSet.h>

#include <tracking/ckf/cdc/entities/CDCCKFState.h>

#include <tracking/dataobjects/RecoTrack.h>

#include <tracking/trackFindingCDC/topology/CDCWire.h>
#include <tracking/trackFindingCDC/topology/CDCWireTopology.h>

using namespace std;
using namespace Belle2;

bool CDCPathBasicVarSet::extract(const BaseCDCPathFilter::Object* path)
{
  // check if hit belongs to same seed
  const auto& seed = path->front();
  const auto* seedRecoTrack = seed.getSeed();

  double arcLength_total = 0.;
  std::vector<double> arcLengths;
  arcLengths.reserve(path->size() - 1);
  std::vector<double> hitDistances;
  hitDistances.reserve(path->size() - 1);
  std::vector<double> flipPos(8, 0);

  unsigned int chargeFlip = 0;
  int lastCharge = seedRecoTrack->getChargeSeed();

  for (auto const& state : *path) {
    if (state.isSeed()) {
      continue;
    }

    arcLengths.push_back(state.getArcLength() - arcLength_total);
    arcLength_total = state.getArcLength();

    hitDistances.push_back(state.getHitDistance());


    // check how many times genfit changed the charge and where it occured
    genfit::MeasuredStateOnPlane trackState = state.getTrackState();
    int stateCharge = trackState.getCharge();
    if (lastCharge != stateCharge) {
      if (chargeFlip < flipPos.size()) {
        flipPos[chargeFlip] = state.getWireHit()->getWire().getICLayer();
      }

      chargeFlip += 1;
      lastCharge = stateCharge;
    }
  }

  // general stuff
  var<named("eventNumber")>() = m_eventMetaData->getEvent();

  // track properties
  var<named("totalHits")>() = path->size() - 1;
  var<named("chargeFlip")>() = chargeFlip;

  bool reachedEnd = false;
  if (path->size() > 1) {
    if (path->back().getWireHit()->getWire().getICLayer() == 0) {
      reachedEnd = true;
    }
  }
  var<named("reachedEnd")>() = reachedEnd;

  var<named("flipPos0")>() = flipPos[0];
  var<named("flipPos1")>() = flipPos[1];
  var<named("flipPos2")>() = flipPos[2];
  var<named("flipPos3")>() = flipPos[3];

  // seed properties
  TVector3 seedPos = seedRecoTrack->getPositionSeed();
  TVector3 seedMom = seedRecoTrack->getMomentumSeed();
  var<named("seed_r")>() = seedPos.Perp();
  var<named("seed_z")>() = seedPos.Z();
  var<named("seed_x")>() = seedPos.X();
  var<named("seed_y")>() = seedPos.Y();
  var<named("seed_p")>() = seedMom.Mag();
  var<named("seed_theta")>() = seedMom.Theta() * 180. / M_PI;
  var<named("seed_pt")>() = seedMom.Perp();
  var<named("seed_pz")>() = seedMom.Z();
  var<named("seed_px")>() = seedMom.X();
  var<named("seed_py")>() = seedMom.Y();
  var<named("seed_charge")>() = seedRecoTrack->getChargeSeed();

  var<named("totalHitsSeedTrack")>() = seedRecoTrack->getNumberOfCDCHits();

  // get ICLayer assigned to seed (only really defined for ECL seeds)
  const auto& wireTopology = TrackFindingCDC::CDCWireTopology::getInstance();
  const auto& wires = wireTopology.getWires();
  const float maxForwardZ = wires.back().getForwardZ();     // 157.615
  const float maxBackwardZ = wires.back().getBackwardZ();   // -72.0916

  int seedICLayer = -1;
  const float seedPosZ = seedPos.z();
  if (seedPosZ < maxForwardZ && seedPosZ > maxBackwardZ) {
    seedICLayer = 56;
  } else {
    // do straight extrapolation of seed momentum to CDC outer walls
    TVector3 seedMomZOne(seedMom * (1. / seedMom.Z()));

    // find closest iCLayer
    float minDist = 99999;
    for (const auto& wire : wires) {
      const float maxZ = seedPosZ > 0 ? wire.getForwardZ() : wire.getBackwardZ();

      const auto distance = wire.getDistance(TrackFindingCDC::Vector3D(seedPos - seedMomZOne * (seedPosZ - maxZ)));
      if (distance < minDist) {
        minDist = distance;
        seedICLayer = wire.getICLayer();
      }
    }
  }
  var<named("seedICLayer")>() = seedICLayer;


  // track representation
  TVector3 trackMom(0, 0, 0);
  int trackCharge = 0;
  float firstChi2 = 0;
  float lastChi2 = 0;
  float firstICLayer = 0;
  float lastICLayer = 0;
  if (path->size() > 1) {
    genfit::MeasuredStateOnPlane trackState = path->back().getTrackState();

    trackMom = trackState.getMom();
    trackCharge = trackState.getCharge();

    firstChi2 = path->at(1).getChi2();
    lastChi2 = path->back().getChi2();

    firstICLayer = path->at(1).getWireHit()->getWire().getICLayer();
    lastICLayer = path->back().getWireHit()->getWire().getICLayer();
  }
  var<named("track_theta")>() = trackMom.Theta() * 180. / M_PI;
  var<named("track_p")>() = trackMom.Mag();
  var<named("track_pt")>() = trackMom.Perp();
  var<named("track_pz")>() = trackMom.Z();
  var<named("track_px")>() = trackMom.X();
  var<named("track_py")>() = trackMom.Y();
  var<named("track_charge")>() = trackCharge;

  var<named("firstChi2")>() = firstChi2;
  var<named("lastChi2")>() = lastChi2;

  var<named("firstICLayer")>() = firstICLayer;
  var<named("lastICLayer")>() = lastICLayer;

  if (path->size() > 3) {
    int lastN = path->size() - 1;
    var<named("ICLayerLast0")>() = path->at(lastN).getWireHit()->getWire().getICLayer();
    var<named("ICLayerLast1")>() = path->at(lastN - 1).getWireHit()->getWire().getICLayer();
    var<named("ICLayerLast2")>() = path->at(lastN - 2).getWireHit()->getWire().getICLayer();
    var<named("IWireLast0")>() = path->at(lastN).getWireHit()->getWire().getIWire();
    var<named("IWireLast1")>() = path->at(lastN - 1).getWireHit()->getWire().getIWire();
    var<named("IWireLast2")>() = path->at(lastN - 2).getWireHit()->getWire().getIWire();
  } else {
    var<named("ICLayerLast0")>() = -1;
    var<named("ICLayerLast1")>() = -1;
    var<named("ICLayerLast2")>() = -1;
    var<named("IWireLast0")>() = -1;
    var<named("IWireLast1")>() = -1;
    var<named("IWireLast2")>() = -1;
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
  } else {
    var<named("hitDistance0")>() = -1;
    var<named("hitDistance1")>() = -1;
    var<named("hitDistance2")>() = -1;
    var<named("hitDistance3")>() = -1;
    var<named("hitDistance4")>() = -1;
    var<named("hitDistance5")>() = -1;
    var<named("hitDistance6")>() = -1;
    var<named("hitDistance7")>() = -1;
    var<named("hitDistance8")>() = -1;
    var<named("hitDistance9")>() = -1;
    var<named("arcLength0")>() = -1;
    var<named("arcLength1")>() = -1;
    var<named("arcLength2")>() = -1;
    var<named("arcLength3")>() = -1;
    var<named("arcLength4")>() = -1;
    var<named("arcLength5")>() = -1;
    var<named("arcLength6")>() = -1;
    var<named("arcLength7")>() = -1;
    var<named("arcLength8")>() = -1;
    var<named("arcLength9")>() = -1;
  }

  return true;
}

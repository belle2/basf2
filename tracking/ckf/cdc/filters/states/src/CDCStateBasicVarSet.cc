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
  var<named("i_hit")>() = path->size() - 1;

  var<named("iCLayer_lastState")>() = lastState.isSeed() ? -1 : lastState.getWireHit()->getWire().getICLayer();
  var<named("iCLayer")>() = state->getWireHit()->getWire().getICLayer();
  var<named("arcLength")>() = state->getArcLength() - lastState.getArcLength();
  var<named("hitDistance")>() = state->getHitDistance();

  TrackFindingCDC::Vector3D wirePos = state->getWireHit()->getRefPos3D();
  var<named("wire_r")>() = wirePos.cylindricalR();
  var<named("wire_z")>() = wirePos.z();
  var<named("wire_x")>() = wirePos.x();
  var<named("wire_y")>() = wirePos.y();

  TVector3 seedPos = seedRecoTrack->getPositionSeed();
  TVector3 seedMom = seedRecoTrack->getMomentumSeed();
  var<named("seed_theta")>() = seedMom.Theta() * 180. / M_PI;
  var<named("seed_r")>() = seedPos.Perp();
  var<named("seed_z")>() = seedPos.Z();
  var<named("seed_x")>() = seedPos.X();
  var<named("seed_y")>() = seedPos.Y();
  var<named("seed_p")>() = seedMom.Mag();
  var<named("seed_pt")>() = seedMom.Perp();
  var<named("seed_pz")>() = seedMom.Z();
  var<named("seed_px")>() = seedMom.X();
  var<named("seed_py")>() = seedMom.Y();
  var<named("seed_charge")>() = seedRecoTrack->getChargeSeed();
  /*
    // track representation
    genfit::MeasuredStateOnPlane trackState = state->getTrackState();
    TVector3 trackPos = trackState.getPos();
    TVector3 trackMom = trackState.getMom();
    var<named("track_momTheta")>() = seedMom.Theta() * 180. / M_PI;
    var<named("track_p")>() = trackMom.Mag();
    var<named("track_pt")>() = trackMom.Perp();
    var<named("track_pz")>() = trackMom.Z();
    var<named("track_px")>() = trackMom.X();
    var<named("track_py")>() = trackMom.Y();
    var<named("track_charge")>() = trackState.getCharge();
    var<named("track_posTheta")>() = trackPos.Theta() * 180. / M_PI;
    var<named("track_r")>() = trackPos.Perp();
    var<named("track_z")>() = trackPos.Z();
    var<named("track_x")>() = trackPos.X();
    var<named("track_y")>() = trackPos.Y();

    genfit::MeasuredStateOnPlane oldTrackState = trackState;
    if (path->size() > 1) {
      oldTrackState = path->at(1).getTrackState();
    }
    TVector3 oldTrackMom = oldTrackState.getMom();
    TVector3 oldTrackPos = oldTrackState.getPos();

    var<named("track_pt_firstHit")>() = oldTrackMom.Perp();
    var<named("track_pz_firstHit")>() = oldTrackMom.Z();
    var<named("track_r_firstHit")>() = oldTrackPos.Perp();
    var<named("track_z_firstHit")>() = oldTrackPos.Z();
  */
  return true;
}

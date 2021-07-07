/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/ckf/cdc/filters/states/CDCStateBasicVarSet.h>

#include <tracking/ckf/cdc/entities/CDCCKFState.h>

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

  return true;
}

/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <alignment/modules/SetRecoTrackMomentum/SetRecoTrackMomentumModule.h>

#include <framework/geometry/BFieldManager.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SetRecoTrackMomentum)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SetRecoTrackMomentumModule::SetRecoTrackMomentumModule() : Module()
{
  // Set module properties
  setDescription(R"DOC("Set momentum magnitude for RecoTracks seed to given value (for runs without magnetic field)

  Take the momentum direction from seed and update its magnitude to artificial value for all RecoTracks - needed for tracks without magnetic field.
  By default activated automatically, when zero B-field is detected at the origin
  )DOC");

  // Parameter definitions
  addParam("automatic", m_automatic, "Detect the B-field at origin automatically - disable module if non-zero", true);
  addParam("momentum", m_momentum, "Default momentum magnitude (GeV/c) to set for seed of RecoTracks", 10.);

}

void SetRecoTrackMomentumModule::initialize()
{
  m_tracks.isRequired();
}

void SetRecoTrackMomentumModule::event()
{
  // In automatic mode, do nothing if B-field > 0 at origin
  if (m_automatic && BFieldManager::getInstance().getField(B2Vector3D(0., 0., 0.)).Mag() > 1.e-14) {
    return;
  }

  for (auto& track : m_tracks) {
    B2Vector3D mom = track.getMomentumSeed();
    mom = 1. / mom.Mag() * m_momentum * mom;

    track.setPositionAndMomentum(track.getPositionSeed(), mom);
  }
}



/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <simulation/kernel/StackingAction.h>
#include <framework/logging/Logger.h>

#include <G4ParticleDefinition.hh>
#include <G4ParticleTypes.hh>
#include <G4Track.hh>
#include <G4RunManager.hh>

using namespace std;
using namespace Belle2;
using namespace Simulation;


StackingAction::StackingAction(): m_photonFraction(1.0)
{

}

StackingAction::~StackingAction()
{

}


G4ClassificationOfNewTrack StackingAction::ClassifyNewTrack(const G4Track* aTrack)
{
  // If optical photon is produced in "Cerenkov" it has "m_photonFraction" probability
  // for being propagated.
  if (aTrack->GetDefinition() == G4OpticalPhoton::OpticalPhotonDefinition()) {
    // particle is optical photon
    if (aTrack->GetParentID() > 0) {
      // particle is secondary
      if (aTrack->GetCreatorProcess()->GetProcessName() == "Cerenkov") {
        // particle is Cerenkov photon
        if (gRandom->Uniform() > m_photonFraction) return fKill;   /**< The random number generator used for rejecting Cerenkov photons.*/
      }
    }
  }
  return fUrgent;
}


void StackingAction::NewStage()
{

}


void StackingAction::PrepareNewEvent()
{

}

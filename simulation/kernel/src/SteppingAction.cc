/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guofu Cao, Andreas Moll                                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <simulation/kernel/SteppingAction.h>
#include <framework/logging/Logger.h>

#include <G4UnitsTable.hh>
#include <G4Track.hh>


using namespace Belle2;
using namespace Simulation;

SteppingAction::SteppingAction()
{
  //Default value for the maximum number of steps
  m_maxNumberSteps = 100000;
}


SteppingAction::~SteppingAction()
{

}


void SteppingAction::UserSteppingAction(const G4Step* step)
{
  G4Track* track = step->GetTrack();

  //------------------------------
  // Check for NULL world volume
  //------------------------------
  if (track->GetVolume() == NULL) {
    B2WARNING("SteppingAction: Track in NULL volume, terminating !\n"
              << "step_no=" + track->GetCurrentStepNumber() << " type=" << track->GetDefinition()->GetParticleName()
              << "\n position=" << G4BestUnit(track->GetPosition(), "Length") << " momentum=" << G4BestUnit(track->GetMomentum(), "Energy"))
    track->SetTrackStatus(fStopAndKill);
    return;
  }

  G4ThreeVector stepPos = step->GetPostStepPoint()->GetPosition();

  //---------------------------------------
  // Check for very high number of steps.
  //---------------------------------------
  if (track->GetCurrentStepNumber() > m_maxNumberSteps) {
    B2WARNING("SteppingAction: Too many steps for this track, terminating !\n"
              << "step_no=" << track->GetCurrentStepNumber() << "type=" << track->GetDefinition()->GetParticleName()
              << "\n position=" << G4BestUnit(track->GetPosition(), "Length") << " momentum=" << G4BestUnit(track->GetMomentum(), "Energy"))
    track->SetTrackStatus(fStopAndKill);
    return;
  }
}

/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <simulation/kernel/SteppingAction.h>
#include <simulation/kernel/UserInfo.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>

#include <G4UnitsTable.hh>
#include <G4Track.hh>


using namespace Belle2;
using namespace Simulation;

SteppingAction::SteppingAction()
{
  //Default value for the maximum number of steps
  m_maxNumberSteps = 100000;
  if (false) {
    G4Step* aStep;
    UserSteppingAction(aStep);
  }
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
    B2WARNING("SteppingAction: Track in NULL volume, terminating!\n"
              << "step_no=" << track->GetCurrentStepNumber() << " type=" << track->GetDefinition()->GetParticleName()
              << "\n position=" << G4BestUnit(track->GetPosition(), "Length") << " momentum=" << G4BestUnit(track->GetMomentum(), "Energy"));
    track->SetTrackStatus(fStopAndKill);
    return;
  }

  //------------------------------
  // Check for absorbers
  //------------------------------
  for (auto& rAbsorber : m_absorbers) {
    const G4ThreeVector stepPrePos = step->GetPreStepPoint()->GetPosition() / CLHEP::mm * Unit::mm;
    const G4ThreeVector stepPostPos = step->GetPostStepPoint()->GetPosition() / CLHEP::mm * Unit::mm;
    if (stepPrePos.perp() < (rAbsorber * Unit::cm) && stepPostPos.perp() > (rAbsorber * Unit::cm)) {
      //B2WARNING("SteppingAction: Track across absorbers, terminating!\n"
      //<< "step_no=" << track->GetCurrentStepNumber() << " type=" << track->GetDefinition()->GetParticleName()
      //<< "\n position=" << G4BestUnit(track->GetPosition(), "Length") << " momentum=" << G4BestUnit(track->GetMomentum(), "Energy") << "\n  PrePos.perp=" << stepPrePos.perp() << ", PostPos.perp=" << stepPostPos.perp() << " cm" );
      track->SetTrackStatus(fStopAndKill);
      return;
    }
  }

  //---------------------------------------
  // Check for very high number of steps.
  //---------------------------------------
  if (track->GetCurrentStepNumber() > m_maxNumberSteps) {
    B2WARNING("SteppingAction: Too many steps for this track, terminating!\n"
              << "step_no=" << track->GetCurrentStepNumber() << "type=" << track->GetDefinition()->GetParticleName()
              << "\n position=" << G4BestUnit(track->GetPosition(), "Length") << " momentum=" << G4BestUnit(track->GetMomentum(), "Energy"));
    track->SetTrackStatus(fStopAndKill);
    return;
  }

  //-----------------------------------------------------------
  // Check if there is an attached trajectory. If so, fill it.
  //-----------------------------------------------------------
  if (m_storeTrajectories) {
    TrackInfo* info = dynamic_cast<TrackInfo*>(track->GetUserInformation());
    if (info && info->getTrajectory()) {
      MCParticleTrajectory& trajectory = *(info->getTrajectory());
      if (trajectory.empty()) {
        const G4ThreeVector stepPos = step->GetPreStepPoint()->GetPosition() / CLHEP::mm * Unit::mm;
        const G4ThreeVector stepMom = step->GetPreStepPoint()->GetMomentum() / CLHEP::MeV * Unit::MeV;
        trajectory.addPoint(
          stepPos.x(), stepPos.y(), stepPos.z(),
          stepMom.x(), stepMom.y(), stepMom.z()
        );
      }
      const G4ThreeVector stepPos = step->GetPostStepPoint()->GetPosition() / CLHEP::mm * Unit::mm;
      const G4ThreeVector stepMom = step->GetPostStepPoint()->GetMomentum() / CLHEP::MeV * Unit::MeV;
      trajectory.addPoint(
        stepPos.x(), stepPos.y(), stepPos.z(),
        stepMom.x(), stepMom.y(), stepMom.z()
      );
    }
  }
}

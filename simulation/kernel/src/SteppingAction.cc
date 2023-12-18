/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <simulation/kernel/SteppingAction.h>
#include <simulation/kernel/EventAction.h>
#include <simulation/kernel/RunManager.h>
#include <simulation/kernel/UserInfo.h>
#include <framework/core/Environment.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>

#include <G4Track.hh>
#include <G4UnitsTable.hh>
#include <G4VProcess.hh>
#include <G4VProcess.hh>

#include <string>

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
  m_writeSimSteps = Environment::Instance().getWriteSimSteps();
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

  // If we are running this job for producing virtual reality events, let's run the relevant method
  if (m_writeSimSteps) {
    writeVREventStep(step, track);
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

// Write (almost) each step to the VR event file
void SteppingAction::writeVREventStep(const G4Step* step, const G4Track* track)
{
  // There must be an open output file (opened in simulation's EventAction)
  RunManager& runManager = RunManager::Instance();
  const EventAction* eventAction = (const Belle2::Simulation::EventAction*)runManager.GetUserEventAction();
  if (eventAction == nullptr)
    return;
  std::ofstream* output = eventAction->getVREventStream();
  if (output == nullptr)
    return;
  if (!output->is_open())
    return;
  // Ignore (hyper)nuclei heavier than alphas
  if (abs(track->GetDefinition()->GetPDGEncoding()) > 1000020040)
    return;
  // Limit the VR event history to 100 ns
  G4StepPoint* postStepPoint = step->GetPostStepPoint();
  if (postStepPoint->GetGlobalTime() > 100.0)
    return;
  // Discard soft particles (KE < 500 keV) unless they are optical photons (for which KE=0)
  G4StepPoint* preStepPoint = step->GetPreStepPoint();
  double KE = preStepPoint->GetTotalEnergy() - track->GetDefinition()->GetPDGMass();
  if ((KE < 0.0005) && (track->GetDefinition()->GetParticleName() != "opticalphoton"))
    return;

  // We will definitely write one record to the VR event file
  G4String pVolName = track->GetVolume()->GetName();
  G4String sensitiveDetectorName = "";
  if (pVolName.compare(0, 4, "PXD.") == 0) {
    if (pVolName.find(".Active") != std::string::npos) { sensitiveDetectorName = "PXD"; }
  } else if (pVolName.compare(0, 4, "SVD.") == 0) {
    if (pVolName.find(".Active") != std::string::npos) { sensitiveDetectorName = "SVD"; }
  } else if (pVolName.compare(0, 20, "physicalSD_CDCLayer_") == 0) {
    sensitiveDetectorName = "CDC";
  } else if (pVolName.compare(0, 19, "TOP.moduleSensitive") == 0) {
    sensitiveDetectorName = "TOP";
  } else if (pVolName.compare(0, 23, "av_1_impr_1_cuttest_pv_") == 0) {
    sensitiveDetectorName = "TOP";
  } else if (pVolName.compare(0, 12, "moduleWindow") == 0) {
    sensitiveDetectorName = "ARICH";
  } else if (pVolName.compare(0, 25, "ARICH.AerogelSupportPlate") == 0) {
    sensitiveDetectorName = "ARICH";
  } else if (pVolName.compare(0, 25, "eclBarrelCrystalPhysical_") == 0) {
    sensitiveDetectorName = "ECL";
  } else if (pVolName.compare(0, 22, "eclFwdCrystalPhysical_") == 0) {
    sensitiveDetectorName = "ECL";
  } else if (pVolName.compare(0, 22, "eclBwdCrystalPhysical_") == 0) {
    sensitiveDetectorName = "ECL";
  } else if (pVolName.compare(0, 20, "BKLM.ScintActiveType") == 0) {
    sensitiveDetectorName = "BKLM";
  } else if (pVolName.compare(0, 10, "BKLM.Layer") == 0) {
    if (pVolName.find("GasPhysical") != std::string::npos) {
      sensitiveDetectorName = "BKLM";
    }
  } else if (pVolName.compare(0, 15, "StripSensitive_") == 0) {
    sensitiveDetectorName = "EKLM";
  }
// Content of each record:
//   TrackID,ParentID,ParticleName,Mass,Charge,StepNumber,Status,VolumeName,
//   MaterialName,IsFirstStepInVolume,IsLastStepInVolume,EnergyDeposit,
//   ProcessType,ProcessName,PrePointX,PrePointY,PrePointZ,PrePointT,
//   PrePointPX,PrePointPY,PrePointPZ,PrePointE,PostPointX,PostPointY,
//   PostPointZ,PostPointT,PostPointPX,PostPointPY,PostPointPZ,PostPointE
  (*output) << std::fixed << std::setprecision(4)
            << track->GetTrackID() << ","
            << track->GetParentID() << ","
            << track->GetDefinition()->GetParticleName() << ","
            << track->GetDefinition()->GetPDGMass() << ","
            << int(track->GetDefinition()->GetPDGCharge()) << ","
            << track->GetCurrentStepNumber() << ","
            << track->GetTrackStatus() << ","
            << pVolName << ","
            << sensitiveDetectorName << ","
            << track->GetMaterial()->GetName() << ","
            << step->IsFirstStepInVolume() << ","
            << step->IsLastStepInVolume() << ","
            << step->GetTotalEnergyDeposit() << ","
            << postStepPoint->GetProcessDefinedStep()->GetProcessType() << ","
            << postStepPoint->GetProcessDefinedStep()->GetProcessName() << ","
            << preStepPoint->GetPosition().x() << ","
            << preStepPoint->GetPosition().y() << ","
            << preStepPoint->GetPosition().z() << ","
            << preStepPoint->GetGlobalTime() << ","
            << preStepPoint->GetMomentum().x() << ","
            << preStepPoint->GetMomentum().y() << ","
            << preStepPoint->GetMomentum().z() << ","
            << preStepPoint->GetTotalEnergy() << ","
            << postStepPoint->GetPosition().x() << ","
            << postStepPoint->GetPosition().y() << ","
            << postStepPoint->GetPosition().z() << ","
            << postStepPoint->GetGlobalTime() << ","
            << postStepPoint->GetMomentum().x() << ","
            << postStepPoint->GetMomentum().y() << ","
            << postStepPoint->GetMomentum().z() << ","
            << postStepPoint->GetTotalEnergy() << std::endl;
}

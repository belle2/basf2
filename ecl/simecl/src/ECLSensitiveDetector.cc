/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:                                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <ecl/simecl/ECLSensitiveDetector.h>

#include <framework/logging/Logger.h>

#include <G4Step.hh>
#include <G4SteppingManager.hh>
#include <G4SDManager.hh>
#include <G4TransportationManager.hh>
#include <G4FieldManager.hh>
#include <G4MagneticField.hh>

#include <TVector3.h>

using namespace Belle2;

ECLSensitiveDetector::ECLSensitiveDetector(G4String name) : Simulation::SensitiveDetectorBase(name)
{

}

ECLSensitiveDetector::~ECLSensitiveDetector()
{

}

void ECLSensitiveDetector::Initialize(G4HCofThisEvent * HCTE)
{
  // Create a new hit collection

  // Assign a unique ID to the hits collection

  // Attach collections to the hits collection of this event

  // Initialize
}

//-----------------------------------------------------
// Method invoked for every step in sensitive detector
//-----------------------------------------------------
G4bool ECLSensitiveDetector::ProcessHits(G4Step *aStep, G4TouchableHistory *)
{
  // Get deposited energy
  const G4double edep = aStep->GetTotalEnergyDeposit();
  if (edep == 0.) return false;

  // Get step length
  const G4double stepLength = aStep->GetStepLength();
  if (stepLength == 0.) return false;

  // Get step information
  const G4Track &track = *aStep->GetTrack();
  const G4double charge = track.GetDefinition()->GetPDGCharge();
  if (charge == 0.) return false;

  const G4double tof = track.GetGlobalTime();
  if (isnan(tof)) {
    B2ERROR("ECLSensitiveDetector: global time is nan");
    return false;
  }

  //const G4int pid = track.GetDefinition()->GetPDGEncoding();
  //const G4int trackID = track.GetTrackID();

  //const G4VPhysicalVolume &vol = *track.GetVolume();
  //const G4StepPoint &PreStep  = *aStep->GetPreStepPoint();
  //const G4StepPoint &PostStep = *aStep->GetPostStepPoint();
  //const G4ThreeVector &posPreStep  = PreStep.GetPosition();
  //const G4ThreeVector &posPostStep = PostStep.GetPosition();
  //const G4ThreeVector momPreStep  = PreStep.GetMomentum();
  //const G4ThreeVector momPostStep = PostStep.GetMomentum();

  //  double radIn = sqrt(posPreStep[0] * posPreStep[0] + posPreStep[1] * posPreStep[1]);

  //  B2INFO(vol.GetName() << " " << radIn)
  return true;
}


void ECLSensitiveDetector::EndOfEvent(G4HCofThisEvent *)
{

}

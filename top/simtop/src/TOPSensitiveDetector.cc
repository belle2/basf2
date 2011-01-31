/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Petric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <top/simtop/TOPSensitiveDetector.h>
#include <framework/logging/Logger.h>
#include <top/hittop/TOPSimHit.h>

#include <G4Step.hh>
#include <G4SteppingManager.hh>
#include <G4SDManager.hh>
#include <G4TransportationManager.hh>
#include <G4UnitsTable.hh>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <TVector3.h>

using namespace Belle2;
using namespace Simulation;

TOPSensitiveDetector::TOPSensitiveDetector(G4String name) : SensitiveDetectorBase(name)
{

}


TOPSensitiveDetector::~TOPSensitiveDetector()
{

}


void TOPSensitiveDetector::Initialize(G4HCofThisEvent* HCTE)
{

  B2INFO("SensitiveDetector initialized");

}


G4bool TOPSensitiveDetector::ProcessHits(G4Step* aStep, G4TouchableHistory*)
{
  //Get particle ID
  G4Track& track  = *aStep->GetTrack();
  if (track.GetDefinition()->GetParticleName() != "opticalphoton") return false;

  //Get time (check for proper global time)
  const G4double globalTime = track.GetGlobalTime();
  if (isnan(globalTime)) {
    B2ERROR("TOP Sensitive Detector: global time is nan !");
    return false;
  }

  //Get step information
  const G4StepPoint& preStep  = *aStep->GetPreStepPoint();

  //Get world position
  const G4ThreeVector& worldPosition = preStep.GetPosition();

  //Transform to local position
  const G4ThreeVector localPosition = preStep.GetTouchableHandle()->GetHistory()->GetTopTransform().TransformPoint(worldPosition);

  //Get module ID number
  const G4int moduleID = preStep.GetTouchableHandle()->GetReplicaNumber(1);

  //Get photon energy
  const G4double energy = track.GetKineticEnergy() / eV;

  //Get ID of parent particle
  const G4int parentID = track.GetParentID();

  //------------------------------------------------------------
  //                Create TOPSimHit and save it to datastore
  //------------------------------------------------------------

  TVector3 locpos(localPosition.x() / cm, localPosition.y() / cm, localPosition.z() / cm);
  StoreArray<TOPSimHit> hitArray("TOPSimHitArray");
  G4int nentr = hitArray->GetEntries();
  new(hitArray->AddrAt(nentr)) TOPSimHit(moduleID, locpos, globalTime, energy, parentID);

  // after detection photon track is killed
  track.SetTrackStatus(fStopAndKill);

  return true;
}


void TOPSensitiveDetector::EndOfEvent(G4HCofThisEvent*)
{

}


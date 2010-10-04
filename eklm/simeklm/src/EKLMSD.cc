/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <eklm/simeklm/EKLMSD.h>
#include <framework/logging/Logger.h>


/*
#ifdef G4VIS_USE
#include "G4VVisManager.hh"
#include "G4Circle.hh"
#include "G4Colour.hh"
#include "G4VisAttributes.hh"
#endif
*/

#include "G4Step.hh"
#include "G4SteppingManager.hh"
#include "G4SDManager.hh"
#include "G4TransportationManager.hh"
#include "G4FieldManager.hh"
#include "G4MagneticField.hh"


using namespace Belle2;

EKLMSD::EKLMSD(G4String name, G4double thresholdEnergyDeposit, G4double thresholdKineticEnergy):
    G4VSensitiveDetector(name), m_ThresholdEnergyDeposit(thresholdEnergyDeposit),
    m_ThresholdKineticEnergy(thresholdKineticEnergy), m_HitCollection(0),
    m_HCID(-1)
{

  G4String CollName1 = name + "Collection";
  collectionName.insert(CollName1);
}

void EKLMSD::Initialize(G4HCofThisEvent * HCTE)
{
  // Create a new hit collection
  m_HitCollection = new EKLMHitsCollection(SensitiveDetectorName, collectionName[0]);

  // Assign a unique ID to the hits collection
  if (m_HCID < 0) {
    m_HCID = G4SDManager::GetSDMpointer()->GetCollectionID(m_HitCollection);
  }

  // Attach collections to HitsCollectionsOfThisEvent
  HCTE -> AddHitsCollection(m_HCID, m_HitCollection);

}

//-----------------------------------------------------
// Method invoked for every step in sensitive detector
//-----------------------------------------------------
G4bool EKLMSD::ProcessHits(G4Step *aStep, G4TouchableHistory *)
{
  // Get deposited energy
  const G4double eDep = aStep->GetTotalEnergyDeposit();
  if (eDep == 0.) return false;

  // Get step length
  const G4double stepLength = aStep->GetStepLength();
  if (stepLength == 0.) return false;

  // Get step information
  const G4Track & t = * aStep->GetTrack();
  const G4double charge = t.GetDefinition()->GetPDGCharge();
  if (charge == 0.) return false;

  const G4double tof = t.GetGlobalTime();
  if (isnan(tof)) {
    ERROR("EKLMSD: global time is nan");
    return false;
  }

  const G4int pid = t.GetDefinition()->GetPDGEncoding();
  const G4int trackID = t.GetTrackID();

  const G4VPhysicalVolume & v = * t.GetVolume();
  const G4StepPoint & in = * aStep->GetPreStepPoint();
  const G4StepPoint & out = * aStep->GetPostStepPoint();
  const G4ThreeVector & posIn = in.GetPosition();
  const G4ThreeVector & posOut = out.GetPosition();
  const G4ThreeVector momIn(in.GetMomentum().x(), in.GetMomentum().y(),
                            in.GetMomentum().z());

  return true;
}


void EKLMSD::EndOfEvent(G4HCofThisEvent *)
{
}

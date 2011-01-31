/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Petric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <top/simtop/TOPSensitiveQuartz.h>
#include <top/geotop/TOPGeometryPar.h>
#include <top/hittop/TOPQuartzHit.h>
#include <framework/logging/Logger.h>

#include <G4Step.hh>
#include <G4SteppingManager.hh>
#include <G4SDManager.hh>
#include <G4TransportationManager.hh>
#include <G4UnitsTable.hh>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>

#include <TVector3.h>
#include <cmath>


using namespace Belle2;
using namespace Simulation;

TOPSensitiveQuartz::TOPSensitiveQuartz(G4String name) : SensitiveDetectorBase(name)
{

}


TOPSensitiveQuartz::~TOPSensitiveQuartz()
{

}


void TOPSensitiveQuartz::Initialize(G4HCofThisEvent* HCTE)
{
  B2INFO("SensitiveQUARTZ initialized");
}


G4bool TOPSensitiveQuartz::ProcessHits(G4Step* aStep, G4TouchableHistory*)
{
  // Get track parameters

  G4Track* aTrack = aStep->GetTrack();
  G4StepPoint* PrePosition =  aStep->GetPreStepPoint();
  G4ThreeVector worldPosition = PrePosition->GetPosition();
  G4ParticleDefinition* particle = aTrack->GetDefinition();
  G4double  PDGCharge = particle->GetPDGCharge();
  G4ThreeVector momentum = PrePosition->GetMomentum();

  // Save only tracks of charged particles
  if (PDGCharge == 0) return(true);

  // Track parameters are saved at the entrance into the quartz
  if (((PrePosition->GetStepStatus() == fGeomBoundary))) {

    G4int  PDGEncoding = particle->GetPDGEncoding();
    G4ThreeVector localPosition = PrePosition->GetTouchableHandle()->GetHistory()->GetTopTransform().TransformPoint(worldPosition);

    // Tracks are saved in "QuartzHitTOPArray"
    StoreArray<TOPQuartzHit> trackArray("QuartzHitTOPArray");
    G4int nentr = trackArray->GetEntries();
    new(trackArray->AddrAt(nentr)) TOPQuartzHit();
    trackArray[nentr]->setTrackID(aTrack->GetTrackID());
    trackArray[nentr]->setParticleID(PDGEncoding);
    TVector3 TPosition(worldPosition.x() / cm, worldPosition.y() / cm, worldPosition.z() / cm);
    trackArray[nentr]->setPosition(TPosition);
    TVector3 TMomentum(momentum.x(), momentum.y(), momentum.z());
    trackArray[nentr]->setMomentum(TMomentum);

    return true;
  }
  return false;
}

void TOPSensitiveQuartz::EndOfEvent(G4HCofThisEvent*)
{

}

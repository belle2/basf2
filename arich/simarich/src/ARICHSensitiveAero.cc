/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <arich/simarich/ARICHSensitiveAero.h>
#include <arich/geoarich/ARICHGeometryPar.h>
#include <arich/hitarich/ARICHAeroHit.h>
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

ARICHSensitiveAero::ARICHSensitiveAero(G4String name) : SensitiveDetectorBase(name)
{
}


ARICHSensitiveAero::~ARICHSensitiveAero()
{

}


void ARICHSensitiveAero::Initialize(G4HCofThisEvent* HCTE)
{
}


G4bool ARICHSensitiveAero::ProcessHits(G4Step* aStep, G4TouchableHistory*)
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

  // Track parameters are saved at the entrance in aerogel
  if (((PrePosition->GetStepStatus() == fGeomBoundary)) && (momentum.z() > 0)) {

    /*       B2INFO ("SensAero: " << aTrack->GetDefinition()->GetParticleName()
        << " " << aTrack->GetTrackID()
        << " " << aTrack->GetParentID()
        << " " << G4BestUnit(worldPosition,"Length")
        << " " << G4BestUnit(aTrack->GetMomentum(), "Energy")
        << " " << G4BestUnit(aTrack->GetGlobalTime(), "Time")
        << " Edep is " << G4BestUnit(aStep->GetTotalEnergyDeposit(),"Energy"));
    */
    G4int  PDGEncoding = particle->GetPDGEncoding();
    G4ThreeVector localPosition = PrePosition->GetTouchableHandle()->GetHistory()->GetTopTransform().TransformPoint(worldPosition);

    // Tracks are saved in "AeroHitARICHArray"
    StoreArray<ARICHAeroHit> trackArray("AeroHitARICHArray");
    G4int nentr = trackArray->GetEntries();
    new(trackArray->AddrAt(nentr)) ARICHAeroHit();
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

void ARICHSensitiveAero::EndOfEvent(G4HCofThisEvent*)
{

}

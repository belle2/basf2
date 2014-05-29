/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Igal Jaegle                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <beast/microtpc/simulation/SensitiveDetector.h>
#include <beast/microtpc/dataobjects/MicrotpcSimHit.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/gearbox/Unit.h>

#include <G4Track.hh>
#include <G4Step.hh>

namespace Belle2 {
  /** Namespace to encapsulate code needed for the MICROTPC detector */
  namespace microtpc {

    SensitiveDetector::SensitiveDetector():
      Simulation::SensitiveDetectorBase("MicrotpcSensitiveDetector", Const::invalidDetector)
    {
      //Make sure all collections are registered
      StoreArray<MCParticle>   mcParticles;
      StoreArray<MicrotpcSimHit>  simHits;
      RelationArray relMCSimHit(mcParticles, simHits);

      //Register all collections we want to modify and require those we want to use
      mcParticles.registerAsPersistent();
      simHits.registerAsPersistent();
      relMCSimHit.registerAsPersistent();

      //Register the Relation so that the TrackIDs get replaced by the actual
      //MCParticle indices after simulating the events. This is needed as
      //secondary particles might not be stored so everything relating to those
      //particles will be attributed to the last saved mother particle
      registerMCParticleRelation(relMCSimHit);
    }

    bool SensitiveDetector::step(G4Step* step, G4TouchableHistory*)
    {
      //Get Track information
      const G4Track& track    = *step->GetTrack();
      const int trackID       = track.GetTrackID();
      const double depEnergy  = step->GetTotalEnergyDeposit() * Unit::MeV;
      const double nielEnergy = step->GetNonIonizingEnergyDeposit() * Unit::MeV;
      const G4ThreeVector G4tkPos = step->GetTrack()->GetPosition();
      TVector3 tkPos(G4tkPos.x() * Unit::cm, G4tkPos.y() * Unit::cm, G4tkPos.z() * Unit::cm);
      const G4ThreeVector G4tkMom = step->GetTrack()->GetMomentum();
      TVector3 tkMom(G4tkMom.x() * Unit::MeV, G4tkMom.y() * Unit::MeV, G4tkMom.z() * Unit::MeV);
      const G4ThreeVector G4tkMomDir = step->GetTrack()->GetMomentumDirection();
      TVector3 tkMomDir(G4tkMomDir.x() * Unit::MeV, G4tkMomDir.y() * Unit::MeV, G4tkMomDir.z() * Unit::MeV);
      const int tkPDG = step->GetTrack()->GetDefinition()->GetPDGEncoding();
      const double tkKEnergy = step->GetTrack()->GetKineticEnergy();
      const int detNb = step->GetTrack()->GetVolume()->GetCopyNo();
      const double GlTime = step->GetPreStepPoint()->GetGlobalTime();
      //Ignore everything below 1eV
      if (depEnergy < Unit::eV) return false;

      //Get the datastore arrays
      StoreArray<MCParticle>  mcParticles;
      StoreArray<MicrotpcSimHit> simHits;
      RelationArray relMCSimHit(mcParticles, simHits);

      //Add SimHit
      const int hitIndex = simHits.getEntries();
      new(simHits.nextFreeAddress()) MicrotpcSimHit(
        depEnergy,
        nielEnergy,
        tkPos,
        tkMom,
        tkMomDir,
        tkPDG,
        tkKEnergy,
        detNb,
        GlTime
      );

      //Add Relation between SimHit and MCParticle with a weight of 1. Since
      //the MCParticle index is not yet defined we use the trackID from Geant4
      relMCSimHit.add(trackID, hitIndex, 1.0);

      return true;
    }

  } //microtpc namespace
} //Belle2 namespace

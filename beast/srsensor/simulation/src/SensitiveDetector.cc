/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Igal Jaegle                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <beast/srsensor/simulation/SensitiveDetector.h>
#include <beast/srsensor/dataobjects/SrsensorSimHit.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/gearbox/Unit.h>

#include <G4Track.hh>
#include <G4Step.hh>

namespace Belle2 {
  /** Namespace to encapsulate code needed for the SRSENSOR detector */
  namespace srsensor {

    SensitiveDetector::SensitiveDetector():
      Simulation::SensitiveDetectorBase("SrsensorSensitiveDetector", Const::invalidDetector)
    {
      //Make sure all collections are registered
      StoreArray<MCParticle>   mcParticles;
      StoreArray<SrsensorSimHit>  simHits;
      RelationArray relMCSimHit(mcParticles, simHits);

      //Register all collections we want to modify and require those we want to use
      mcParticles.registerInDataStore();
      simHits.registerInDataStore();
      relMCSimHit.registerInDataStore();

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
      float tkPos[3];
      tkPos[0] = G4tkPos.x() * Unit::cm;
      tkPos[1] = G4tkPos.y() * Unit::cm;
      tkPos[2] = G4tkPos.z() * Unit::cm;
      const G4ThreeVector G4tkMom = step->GetTrack()->GetMomentum();
      float tkMom[3];
      tkMom[0] = G4tkMom.x() * Unit::MeV;
      tkMom[1] = G4tkMom.y() * Unit::MeV;
      tkMom[2] = G4tkMom.z() * Unit::MeV;
      const G4ThreeVector G4tkMomDir = step->GetTrack()->GetMomentumDirection();
      float tkMomDir[3];
      tkMomDir[0] = G4tkMomDir.x() * Unit::MeV;
      tkMomDir[1] = G4tkMomDir.y() * Unit::MeV;
      tkMomDir[2] = G4tkMomDir.z() * Unit::MeV;
      const int tkPDG = step->GetTrack()->GetDefinition()->GetPDGEncoding();
      const double tkKEnergy = step->GetTrack()->GetKineticEnergy();
      const int detNb = step->GetTrack()->GetVolume()->GetCopyNo();
      const double GlTime = step->GetPreStepPoint()->GetGlobalTime();
      //Ignore everything below 1eV
      if (depEnergy < Unit::eV) return false;

      //Get the datastore arrays
      StoreArray<MCParticle>  mcParticles;
      StoreArray<SrsensorSimHit> simHits;
      RelationArray relMCSimHit(mcParticles, simHits);

      StoreArray<SrsensorSimHit> SrsensorHits;
      if (!SrsensorHits.isValid()) SrsensorHits.create();
      SrsensorSimHit* hit = SrsensorHits.appendNew(
                              depEnergy,
                              nielEnergy,
                              tkPDG,
                              tkKEnergy,
                              detNb,
                              GlTime,
                              tkPos,
                              tkMom,
                              tkMomDir
                            );

      //Add Relation between SimHit and MCParticle with a weight of 1. Since
      //the MCParticle index is not yet defined we use the trackID from Geant4
      relMCSimHit.add(trackID, hit->getArrayIndex(), 1.0);

      return true;
    }

  } //srsensor namespace
} //Belle2 namespace

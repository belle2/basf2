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
#include <beast/microtpc/dataobjects/TpcMCParticle.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/gearbox/Unit.h>

#include <G4Track.hh>
#include <G4Step.hh>
#include "G4VProcess.hh"

namespace Belle2 {
  /** Namespace to encapsulate code needed for the MICROTPC detector */
  namespace microtpc {

    SensitiveDetector::SensitiveDetector():
      Simulation::SensitiveDetectorBase("MicrotpcSensitiveDetector", Const::invalidDetector)
    {
      //Make sure all collections are registered
      StoreArray<MCParticle>   mcParticles;
      StoreArray<MicrotpcSimHit>  simHits;
      StoreArray<TpcMCParticle> TpcMCParticles;
      RelationArray relMCSimHit(mcParticles, simHits);

      //Register all collections we want to modify and require those we want to use
      mcParticles.registerInDataStore();
      simHits.registerInDataStore();
      relMCSimHit.registerInDataStore();
      TpcMCParticles.registerInDataStore();

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
      const double depEnergy  = step->GetTotalEnergyDeposit() * CLHEP::MeV;
      const double nielEnergy = step->GetNonIonizingEnergyDeposit() * CLHEP::MeV;
      const G4ThreeVector G4tkPos = step->GetTrack()->GetPosition();
      float tkPos[3];
      tkPos[0] = G4tkPos.x() * CLHEP::cm;
      tkPos[1] = G4tkPos.y() * CLHEP::cm;
      tkPos[2] = G4tkPos.z() * CLHEP::cm;
      const G4ThreeVector G4tkMom = step->GetTrack()->GetMomentum();
      float tkMom[3];
      tkMom[0] = G4tkMom.x() * CLHEP::MeV;
      tkMom[1] = G4tkMom.y() * CLHEP::MeV;
      tkMom[2] = G4tkMom.z() * CLHEP::MeV;
      const G4ThreeVector G4tkMomDir = step->GetTrack()->GetMomentumDirection();
      float tkMomDir[3];
      tkMomDir[0] = G4tkMomDir.x() * CLHEP::MeV;
      tkMomDir[1] = G4tkMomDir.y() * CLHEP::MeV;
      tkMomDir[2] = G4tkMomDir.z() * CLHEP::MeV;
      const int tkPDG = step->GetTrack()->GetDefinition()->GetPDGEncoding();
      const double tkKEnergy = step->GetTrack()->GetKineticEnergy();
      const int detNb = step->GetTrack()->GetVolume()->GetCopyNo();
      const double GlTime = step->GetPreStepPoint()->GetGlobalTime();
      //Ignore everything below 1eV
      if (depEnergy < CLHEP::eV) return false;

      //Get the datastore arrays
      StoreArray<MCParticle>  mcParticles;
      StoreArray<MicrotpcSimHit> simHits;
      RelationArray relMCSimHit(mcParticles, simHits);

      //find out if the process that created the particle was a neutron process
      bool neuProc = false;
      G4String CPName;
      if (step->GetTrack()->GetCreatorProcess() != 0) {
        const  G4VProcess* creator = step->GetTrack()->GetCreatorProcess();
        CPName = creator->GetProcessName();
        if (CPName.contains("Neutron")) neuProc = true;
      }
      //Save Hit if track leaves volume or is killed
      if (track.GetNextVolume() != track.GetVolume() || track.GetTrackStatus() >= fStopAndKill) {
        if (neuProc) saveSimHit();
      }

      StoreArray<MicrotpcSimHit> MicrotpcHits;
      if (!MicrotpcHits.isValid()) MicrotpcHits.create();
      MicrotpcSimHit* hit = MicrotpcHits.appendNew(
                              trackID,
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

    int SensitiveDetector::saveSimHit()
    {

      //Get the datastore arrays

      StoreArray<MCParticle> mcParticles;
      StoreArray<TpcMCParticle> TpcMCParticles;
      for (const auto& mcParticle : mcParticles) { // start loop over all Tracks
        int pdg = mcParticle.getPDG();
        if (pdg == 2112) {
          int PDG = mcParticle.getPDG();
          float Mass = mcParticle.getMass();
          float Energy = mcParticle.getEnergy();
          float vtx[3];
          vtx[0] = mcParticle.getProductionVertex().X();
          vtx[1] = mcParticle.getProductionVertex().Y();
          vtx[2] = mcParticle.getProductionVertex().Z();
          float mom[3];
          mom[0] = mcParticle.getMomentum().X();
          mom[1] = mcParticle.getMomentum().Y();
          mom[2] = mcParticle.getMomentum().Z();

          if (!TpcMCParticles.isValid()) TpcMCParticles.create();
          TpcMCParticles.appendNew(TpcMCParticle(PDG, Mass, Energy, vtx, mom));
        }
      }
      return (m_simhitNumber);
    }//saveSimHit

  } //microtpc namespace
} //Belle2 namespace

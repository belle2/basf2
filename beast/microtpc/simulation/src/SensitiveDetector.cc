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
#include <beast/microtpc/dataobjects/TPCG4TrackInfo.h>

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
      m_trackID = 0;
      //Make sure all collections are registered
      StoreArray<MCParticle>   mcParticles;
      StoreArray<MicrotpcSimHit>  simHits;
      StoreArray<TPCG4TrackInfo> TPCG4TrackInfos;
      RelationArray relMCSimHit(mcParticles, simHits);

      //Register all collections we want to modify and require those we want to use
      mcParticles.registerInDataStore();
      simHits.registerInDataStore();
      relMCSimHit.registerInDataStore();
      TPCG4TrackInfos.registerInDataStore();

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
      //if (depEnergy < CLHEP::eV) return false;

      //Get the datastore arrays
      StoreArray<MCParticle>  mcParticles;
      StoreArray<MicrotpcSimHit> simHits;
      RelationArray relMCSimHit(mcParticles, simHits);
      /*
      //find out if the process that created the particle was a neutron process
      bool neuProc = false;
      G4String CPName;
      if (step->GetTrack()->GetCreatorProcess() != 0) {
        const  G4VProcess* creator = step->GetTrack()->GetCreatorProcess();
        CPName = creator->GetProcessName();
        if (CPName.contains("Neutron")) neuProc = true;
      }
      */
      if (m_trackID != track.GetTrackID()) {
        //TrackID changed, store track informations
        m_trackID = track.GetTrackID();
      }

      //Save Hit if track leaves volume or is killed
      if (track.GetNextVolume() != track.GetVolume() || track.GetTrackStatus() >= fStopAndKill) {
        auto& mcparticle = Simulation::TrackInfo::getInfo(track);
        int PDG = mcparticle.getPDG();
        float Mass = mcparticle.getMass();
        float Energy = mcparticle.getEnergy();
        float vtx[3];
        vtx[0] = mcparticle.getProductionVertex().X();
        vtx[1] = mcparticle.getProductionVertex().Y();
        vtx[2] = mcparticle.getProductionVertex().Z();
        float mom[3];
        mom[0] = mcparticle.getMomentum().X();
        mom[1] = mcparticle.getMomentum().Y();
        mom[2] = mcparticle.getMomentum().Z();
        saveG4TrackInfo(m_trackID, PDG, Mass, Energy, vtx, mom);
        //Reset TrackID
        m_trackID = 0;
      }

      StoreArray<MicrotpcSimHit> MicrotpcHits;
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

    int SensitiveDetector::saveG4TrackInfo
    (
      int trackID,
      int PDG,
      float Mass,
      float Energy,
      float vtx[3],
      float mom[3]
    )
    {
      //Get the datastore arrays
      StoreArray<TPCG4TrackInfo> TPCG4TrackInfos;

      TPCG4TrackInfos.appendNew(TPCG4TrackInfo(trackID, PDG, Mass, Energy, vtx, mom));

      int simhitNumber = TPCG4TrackInfos.getEntries() - 1;

      return (simhitNumber);
    }//saveG4TrackInfo

  } //microtpc namespace
} //Belle2 namespace

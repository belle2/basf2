/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Igal Jaegle                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <beast/bgo/simulation/SensitiveDetector.h>
#include <beast/bgo/dataobjects/BgoSimHit.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/gearbox/Unit.h>

#include <G4Track.hh>
#include <G4Step.hh>

namespace Belle2 {
  /** Namespace to encapsulate code needed for the BGO detector */
  namespace bgo {

    SensitiveDetector::SensitiveDetector():
      Simulation::SensitiveDetectorBase("BgoSensitiveDetector", Const::invalidDetector)
    {
      m_hitNum = 0;
      m_EvnetNumber = 0;
      m_oldEvnetNumber = 0;
      m_trackID = 0;
      m_startTime = 0;
      m_endTime = 0;
      m_WightedTime = 0;
      m_startEnergy = 0;
      m_energyDeposit = 0;
      m_trackLength = 0;
      iECLCell = 0;
      TimeIndex = 0;
      local_pos = 0;
      T_ave = 0;
      firstcall = 0;
      m_phiID = 0;
      m_thetaID = 0;
      m_cellID = 0;

      //Make sure all collections are registered
      StoreArray<MCParticle>   mcParticles;
      StoreArray<BgoSimHit>  simHits;
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

    SensitiveDetector::~SensitiveDetector()
    {

    }

    bool SensitiveDetector::step(G4Step* step, G4TouchableHistory*)
    {
      const G4StepPoint& preStep  = *step->GetPreStepPoint();
      const G4StepPoint& postStep = *step->GetPostStepPoint();

      G4Track& track  = *step->GetTrack();
      if (m_trackID != track.GetTrackID()) {
        //TrackID changed, store track informations
        m_trackID = track.GetTrackID();
        //Get momentum
        m_momentum = preStep.GetMomentum() ;
        //Get energy
        m_startEnergy =  preStep.GetKineticEnergy() ;
        //Reset energy deposit;
        m_energyDeposit = 0;
        //Reset Wighted Time;
        m_WightedTime = 0;
        //Reset m_WightedPos;
        m_WightedPos.SetXYZ(0, 0, 0);

      }
      //Update energy deposit
      m_energyDeposit += step->GetTotalEnergyDeposit() ;

      m_startTime = preStep.GetGlobalTime();
      m_endTime = postStep.GetGlobalTime();
      m_WightedTime += (m_startTime + m_endTime) / 2 * (step->GetTotalEnergyDeposit());

      m_startPos =  preStep.GetPosition();
      m_endPos = postStep.GetPosition();
      TVector3 position((m_startPos.getX() + m_endPos.getX()) / 2 / CLHEP::cm, (m_startPos.getY() + m_endPos.getY()) / 2 / CLHEP::cm,
                        (m_startPos.getZ() + m_endPos.getZ()) / 2 / CLHEP::cm);
      m_WightedPos += position * (step->GetTotalEnergyDeposit());

      //Save Hit if track leaves volume or is killed
      if (track.GetNextVolume() != track.GetVolume() || track.GetTrackStatus() >= fStopAndKill) {
        int pdgCode = track.GetDefinition()->GetPDGEncoding();

        const G4VPhysicalVolume& v = * track.GetVolume();
        G4ThreeVector posCell = v.GetTranslation();
        // Get layer ID

        //if (v.GetName().find("Crystal") != std::string::npos) {
        //CsiGeometryPar* eclp = CsiGeometryPar::Instance();
        m_cellID = step->GetTrack()->GetVolume()->GetCopyNo();

        double dTotalEnergy = 1 / m_energyDeposit; //avoid the error  no match for 'operator/'
        if (m_energyDeposit > 0.) {

          saveSimHit(m_cellID, m_trackID, pdgCode, m_WightedTime / m_energyDeposit,
                     m_energyDeposit, m_momentum, m_WightedPos * dTotalEnergy);
          //}
        }

        //Reset TrackID
        m_trackID = 0;
      }

      return true;
    }
    /*
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
      StoreArray<BgoSimHit> simHits;
      RelationArray relMCSimHit(mcParticles, simHits);

      StoreArray<BgoSimHit> BgoHits;
      if (!BgoHits.isValid()) BgoHits.create();
      BgoSimHit* hit = BgoHits.appendNew(
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
    */

    int SensitiveDetector::saveSimHit(
      const G4int cellId,
      const G4int trackID,
      const G4int pid,
      const G4double tof,
      const G4double edep,
      G4ThreeVector mom,
      TVector3 posAve)
    {

      //Get the datastore arraus
      StoreArray<MCParticle>   mcParticles;
      StoreArray<BgoSimHit>  simHits;
      RelationArray relMBgomHit(mcParticles, simHits);

      StoreArray<BgoSimHit> BgoHits;
      RelationArray bgoSimHitRel(mcParticles, BgoHits);
      TVector3 momentum(mom.getX() / CLHEP::GeV, mom.getY() / CLHEP::GeV, mom.getZ() / CLHEP::GeV);
      BgoHits.appendNew(cellId, trackID, pid, tof / CLHEP::ns, edep / CLHEP::GeV, momentum, posAve);
      int simhitNumber = BgoHits.getEntries() - 1;
      B2DEBUG(150, "HitNumber: " << simhitNumber);
      bgoSimHitRel.add(trackID, simhitNumber);
      return (simhitNumber);
    }//saveSimHit


  } //bgo namespace
} //Belle2 namespace

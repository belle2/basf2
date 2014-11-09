/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Igal Jaegle                               *
 *               Alexandre Beaulieu
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <beast/csi/simulation/SensitiveDetector.h>
#include <beast/csi/dataobjects/CsiSimHit.h>
#include <beast/csi/geometry/CsiGeometryPar.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/gearbox/Unit.h>

#include <G4Track.hh>
#include <G4Step.hh>

namespace Belle2 {
  /** Namespace to encapsulate code needed for the CSI detector */
  namespace csi {

    SensitiveDetector::SensitiveDetector():
      Simulation::SensitiveDetectorBase("CsiSensitiveDetector", Const::invalidDetector)
    {
      m_simhitNumber = 0;
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
      StoreArray<CsiSimHit>  simHits;
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



    bool SensitiveDetector::step(G4Step* aStep, G4TouchableHistory*)
    {

      const G4StepPoint& preStep  = *aStep->GetPreStepPoint();
      const G4StepPoint& postStep = * aStep->GetPostStepPoint();

      G4Track& track  = * aStep->GetTrack();
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
      m_energyDeposit += aStep->GetTotalEnergyDeposit() ;

      m_startTime = preStep.GetGlobalTime();
      m_endTime = postStep.GetGlobalTime();
      m_WightedTime += (m_startTime + m_endTime) / 2 * (aStep->GetTotalEnergyDeposit());

      m_startPos =  preStep.GetPosition();
      m_endPos = postStep.GetPosition();
      TVector3 position((m_startPos.getX() + m_endPos.getX()) / 2 / CLHEP::cm, (m_startPos.getY() + m_endPos.getY()) / 2 / CLHEP::cm, (m_startPos.getZ() + m_endPos.getZ()) / 2 / CLHEP::cm);
      m_WightedPos += position * (aStep->GetTotalEnergyDeposit());

      //Save Hit if track leaves volume or is killed
      if (track.GetNextVolume() != track.GetVolume() || track.GetTrackStatus() >= fStopAndKill) {
        int pdgCode = track.GetDefinition()->GetPDGEncoding();

        const G4VPhysicalVolume& v = * track.GetVolume();
        G4ThreeVector posCell = v.GetTranslation();
        // Get layer ID

        if (v.GetName().find("Crystal") != std::string::npos) {
          CsiGeometryPar* eclp = CsiGeometryPar::Instance();
          m_cellID = eclp->CsiVolNameToCellID(v.GetName());

          double dTotalEnergy = 1 / m_energyDeposit; //avoid the error  no match for 'operator/'
          if (m_energyDeposit > 0.) {
            saveSimHit(m_cellID, m_trackID, pdgCode, m_WightedTime / m_energyDeposit,
                       m_energyDeposit, m_momentum, m_WightedPos * dTotalEnergy);
          }
        }

        //Reset TrackID
        m_trackID = 0;
      }

      return true;
    }

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
      StoreArray<CsiSimHit>  simHits;
      RelationArray relMCSimHit(mcParticles, simHits);

      StoreArray<CsiSimHit> CsiHits;
      if (!CsiHits.isValid()) CsiHits.create();
      RelationArray csiSimHitRel(mcParticles, CsiHits);
      TVector3 momentum(mom.getX() / CLHEP::GeV, mom.getY() / CLHEP::GeV, mom.getZ() / CLHEP::GeV);
      CsiHits.appendNew(cellId, trackID, pid, tof / CLHEP::ns, edep / CLHEP::GeV, momentum, posAve);
      B2DEBUG(150, "HitNumber: " << m_simhitNumber);
      int m_simhitNumber = CsiHits.getEntries() - 1;
      csiSimHitRel.add(trackID, m_simhitNumber);
      return (m_simhitNumber);
    }//saveSimHit

  } //csi namespace
} //Belle2 namespace

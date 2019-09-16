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
#include <beast/csi/dataobjects/CsiHit.h>
#include <framework/dataobjects/EventMetaData.h>
#include <beast/csi/geometry/CsiGeometryPar.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/gearbox/Unit.h>

#include <G4Track.hh>
#include <G4Step.hh>

namespace Belle2 {
  /** Namespace to encapsulate code needed for the CSI detector */
  namespace csi {

    SensitiveDetector::SensitiveDetector():
      Simulation::SensitiveDetectorBase("CsiSensitiveDetector", Const::invalidDetector)
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


      // CsiHitIndex: array of 18 (Nb. crystals) x 80 (max time index (tof [ns]/100))
      for (int i = 0; i < 18; i++) {for (int j = 0; j < 80; j++) { CsiHitIndex[i][j] = 0;}}

      //Make sure all collections are registered
      StoreArray<MCParticle>   mcParticles;
      StoreArray<CsiSimHit>  simHits;
      StoreArray<CsiHit> csiHits;

      RelationArray simHitRel(mcParticles, simHits);
      RelationArray csiHitRel(mcParticles, csiHits);

      //Register all collections we want to modify and require those we want to use
      mcParticles.registerInDataStore();
      simHits.registerInDataStore();
      csiHits.registerInDataStore();
      simHitRel.registerInDataStore();
      csiHitRel.registerInDataStore();

      //Register the Relation so that the TrackIDs get replaced by the actual
      //MCParticle indices after simulating the events. This is needed as
      //secondary particles might not be stored so everything relating to those
      //particles will be attributed to the last saved mother particle
      registerMCParticleRelation(simHitRel);
      registerMCParticleRelation(csiHitRel);
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
        m_WightedPos.set(0, 0, 0);

      }
      //Update energy deposit
      m_energyDeposit += aStep->GetTotalEnergyDeposit() ;

      m_startTime = preStep.GetGlobalTime();
      m_endTime = postStep.GetGlobalTime();
      m_WightedTime += (m_startTime + m_endTime) / 2 * (aStep->GetTotalEnergyDeposit());

      m_startPos =  preStep.GetPosition();
      m_endPos = postStep.GetPosition();

      G4ThreeVector position(0.5 * (m_startPos + m_endPos));
      m_WightedPos += position * (aStep->GetTotalEnergyDeposit());

      //Save Hit if track leaves volume or is killed
      if (track.GetNextVolume() != track.GetVolume() || track.GetTrackStatus() >= fStopAndKill) {
        int pdgCode = track.GetDefinition()->GetPDGEncoding();

        const G4VPhysicalVolume& v = * track.GetVolume();

        //Column z of the rotation matrix R is the result of R * zhat.
        // This is what we want since G4Trap are created aligned along z
        G4RotationMatrix rotCell = * v.GetObjectRotation();
        //        G4RotationMatrix rotCell = * v.GetRotation();
        G4ThreeVector vecCell = rotCell.colZ();
        G4ThreeVector posCell = v.GetTranslation();

        // Get layer ID
        if (v.GetName().find("Crystal") != std::string::npos) {
          CsiGeometryPar* csip = CsiGeometryPar::Instance();
          m_cellID = csip->CsiVolNameToCellID(v.GetName());

          double dTotalEnergy = 1.0 / m_energyDeposit; //avoid the error  no match for 'operator/'
          if (m_energyDeposit > 0.) {
            saveSimHit(m_cellID, m_trackID, pdgCode, m_WightedTime * dTotalEnergy,
                       m_energyDeposit, m_momentum, m_WightedPos * dTotalEnergy,
                       posCell, vecCell);
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
      G4ThreeVector pos,
      G4ThreeVector PosCell,
      G4ThreeVector VecCell)
    {

      //Get the datastore arrays
      StoreArray<MCParticle>   mcParticles;
      StoreArray<CsiSimHit>    simHits;

      RelationArray simHitRel(mcParticles, simHits);

      // convert Hep3Vectors to TVector3
      TVector3 momentum(mom.getX() , mom.getY() , mom.getZ());
      TVector3 position(pos.getX() , pos.getY() , pos.getZ());

      //Append
      simHits.appendNew(cellId, trackID, pid, tof / CLHEP::ns, edep / CLHEP::GeV,
                        momentum * (1.0 / CLHEP::GeV), position * (1.0 / CLHEP::cm));

      int simhitNumber = simHits.getEntries() - 1;
      simHitRel.add(trackID, simhitNumber);
      B2DEBUG(150, "Saving CsiSimHit Number: " << simhitNumber);

      // Store CsiHit: total energy deposited per hit, with average time.

      // Old names:
      /*
      StoreArray<CsiHit> eclHitArray;
      RelationArray eclHitRel(mcParticles, eclHitArray);
      */

      // New names:

      StoreArray<CsiHit> csiHits;

      RelationArray csiHitRel(mcParticles, csiHits);


      StoreObjPtr<EventMetaData> eventMetaDataPtr;
      int m_currentEvnetNumber = eventMetaDataPtr->getEvent();


      if (firstcall == 0 || m_currentEvnetNumber != m_oldEvnetNumber) {
        m_oldEvnetNumber = m_currentEvnetNumber;
        for (int iCSICell = 0; iCSICell < 18; iCSICell++) {
          for (int  TimeInd = 0; TimeInd < 80; TimeInd++) {
            CsiHitIndex[iCSICell][TimeInd] = -1;
          }
        }
        firstcall++;
      }


      if (m_currentEvnetNumber == m_oldEvnetNumber) {
        if ((tof / CLHEP::ns) < 8000) {
          TimeIndex = (int)((tof / CLHEP::ns) / 100);

          // local_pos is the distance between the hit and the PIN-diode end of the crystal.
          local_pos = (15. - (pos  - PosCell) * VecCell);

          // From the ECL package T_ave =  6.05 + 0.0749 * local_pos - 0.00112 * local_pos * local_pos + (tof / CLHEP::ns)  ;

          //For CsI: we remove the offset (I hypothetize it if from photo-sensor delay)
          //         we remove the acceleration of the light
          //         we correct the velocity term using n_CsI=1.80
          T_ave = 0.0600 * local_pos + (tof / CLHEP::ns)  ;


          double E_cell = (edep / CLHEP::GeV);
          if (CsiHitIndex[cellId][TimeIndex] == -1) {
            //m_hitNum = csiHits->GetLast() + 1;
            //new(csiHits->AddrAt(m_hitNum)) CSIHit();
            csiHits.appendNew();
            m_hitNum = csiHits.getEntries() - 1;
            CsiHitIndex[cellId][TimeIndex] = m_hitNum;
            csiHits[m_hitNum]->setCellId(cellId);
            csiHits[m_hitNum]->setEnergyDep(E_cell);
            csiHits[m_hitNum]->setTimeAve(T_ave);
            csiHits[m_hitNum]->setTimeVar(0);
            csiHitRel.add(trackID, m_hitNum);

          } else {
            m_hitNum = CsiHitIndex[cellId][TimeIndex];
            double old_edep = csiHits[m_hitNum]->getEnergyDep();
            double old_TimeAve = csiHits[m_hitNum]->getTimeAve();
            double old_TimeVar = csiHits[m_hitNum]->getTimeVar();

            //double new_edep = E_cell + old_edep;
            //double new_TimeAve = (old_edep * old_TimeAve + E_cell * T_ave) / new_edep;

            double temp = E_cell + old_edep;
            double delta = T_ave - old_TimeAve;
            double R = delta * E_cell / temp;
            double new_TimeAve = old_TimeAve + R;
            double new_TimeVar = old_TimeVar + old_edep * delta * R;
            double new_edep = temp;

            //Algo from Wiki:
            /*
              for x, weight in dataWeightPairs:  # Alternatively "for x, weight in zip(data, weights):"
                  temp = weight + sumweight
                delta = x - mean
                R = delta * weight / temp
                mean = mean + R
                M2 = M2 + sumweight * delta * R  # Alternatively, "M2 = M2 + weight * delta * (x−mean)"
                sumweight = temp

              variance_n = M2/sumweight
            */

            csiHits[m_hitNum]->setEnergyDep(new_edep);
            csiHits[m_hitNum]->setTimeAve(new_TimeAve);
            csiHits[m_hitNum]->setTimeVar(new_TimeVar);
            csiHitRel.add(trackID, m_hitNum);

          }
        }
      } else { B2ERROR("m_currentEvnetNumber  ERROR: m_oldEvnetNumber==m_oldEvnetNumber"); }

      return (simhitNumber);
    }//saveSimHit
  } //csi namespace
} //Belle2 namespace

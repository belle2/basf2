/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Poyuan Chen                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/



#include <ecl/simulation/SensitiveDetector.h>

#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/logging/Logger.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <ecl/dataobjects/ECLSimHit.h>
#include <ecl/dataobjects/ECLHit.h>
#include <ecl/geometry/ECLGeometryPar.h>


#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>

#include <G4TrackingManager.hh>
#include <G4Track.hh>
#include <G4Step.hh>
#include <G4SteppingManager.hh>
#include <G4SDManager.hh>
#include <G4TransportationManager.hh>
#include <G4FieldManager.hh>
#include <G4MagneticField.hh>

#include "CLHEP/Units/PhysicalConstants.h"

//#include "CLHEP/Geometry/Vector3D.h"
//#include "CLHEP/Geometry/Point3D.h"

#include "TVector3.h"
#define PI 3.14159265358979323846

using namespace std;

namespace Belle2 {
  namespace ECL {

    SensitiveDetector::SensitiveDetector(G4String name, G4double thresholdEnergyDeposit, G4double thresholdKineticEnergy):
      Simulation::SensitiveDetectorBase(name, Const::ECL), m_thresholdEnergyDeposit(thresholdEnergyDeposit),
      m_thresholdKineticEnergy(thresholdKineticEnergy)
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


      for (int i = 0; i < 8736; i++) {for (int j = 0; j < 80; j++) { ECLHitIndex[i][j] = 0;}}
      StoreArray<ECLSimHit>eclSimHits;
      StoreArray<ECLHit>eclHits;
      StoreArray<MCParticle>mcParticles;
      RelationArray eclSimHitRel(mcParticles, eclSimHits);
      registerMCParticleRelation(eclSimHitRel);
      RelationArray eclHitRel(mcParticles, eclHits);
      registerMCParticleRelation(eclHitRel);

      // Following lines are obsolete.
      // eclSimHits.registerAsPersistent();
      // eclHits.registerAsPersistent();

      eclSimHits.registerInDataStore();
      eclHits.registerInDataStore();

      mcParticles.registerRelationTo(eclSimHits);
      mcParticles.registerRelationTo(eclHits);

      // Following lines are obsolete.
      // StoreArray<ECLSimHit>::registerPersistent();
      // StoreArray<ECLHit>::registerPersistent();
      // RelationArray::registerPersistent<MCParticle, ECLSimHit>("", "");
      // RelationArray::registerPersistent<MCParticle, ECLHit>("", "");

    }


    SensitiveDetector::~SensitiveDetector()
    {

    }

    void SensitiveDetector::Initialize(G4HCofThisEvent*)
    {}

//-----------------------------------------------------
// Method invoked for every step in sensitive detector
//-----------------------------------------------------
//G4bool SensitiveDetector::ProcessHits(G4Step *aStep, G4TouchableHistory *)
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

        if (v.GetName().find("Crystal") != string::npos) {
          ECLGeometryPar* eclp = ECLGeometryPar::Instance();
          m_cellID = eclp->ECLVolNameToCellID(v.GetName());

          double dTotalEnergy = 1 / m_energyDeposit; //avoid the error  no match for 'operator/'
          if (m_energyDeposit > 0.) saveSimHit(m_cellID, m_trackID, pdgCode, m_WightedTime / m_energyDeposit , m_energyDeposit, m_momentum, m_WightedPos * dTotalEnergy);
        }

        //Reset TrackID
        m_trackID = 0;
      }

      /*
                 if (track.GetNextVolume() != track.GetVolume() || track.GetTrackStatus() >= fStopAndKill) {
                    if(find(myvector.begin(), myvector.end(), track.GetParentID()) !=myvector.end()  ){
                      myvector.push_back(m_trackID);
                      cout<<"saved track "<<m_trackID<<endl;

                    }
                    else if((find(myvector.begin(), myvector.end(), track.GetParentID()) ==myvector.end()&&find(myvector.begin(), myvector.end(), track.GetParentID()->GetParentID()) ==myvector.end()  )||track.GetParentID()==0)
                    {cout<<"Myvector Size "<< myvector.size()<<"  mother "<<track.GetParentID()  <<" first track "<< m_trackID<<" "<<pdgCode<<endl;
                     myvector.clear();
                     myvector.push_back(m_trackID);}
                 }else{
                    const G4VPhysicalVolume& v1 = *track.GetNextVolume();
                   cout<< m_trackID <<" track.GetTrackStatus() "<<track.GetTrackStatus()<<" track.GetNextVolume()   "<<v1.GetName()<<" "<<v.GetName()<<endl;

                }

      */


//    cout << pdgCode << " CellID " << m_cellID  <<" track ID "<<m_trackID<< endl;
      // Ge layer ID
//  const unsigned layerId = v.GetCopyNo();
      return true;
    }


    void SensitiveDetector::EndOfEvent(G4HCofThisEvent*)
    {
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
      StoreArray<MCParticle> mcParticles;
      //change Later
      StoreArray<ECLSimHit> eclArray;
      if (!eclArray) eclArray.create();
      RelationArray eclSimHitRel(mcParticles, eclArray);
      TVector3 momentum(mom.getX() / CLHEP::GeV, mom.getY() / CLHEP::GeV, mom.getZ() / CLHEP::GeV);
      eclArray.appendNew(cellId + 1, trackID, pid, tof / CLHEP::ns, edep / CLHEP::GeV, momentum, posAve);
      /*
            m_simhitNumber = eclArray->GetLast() + 1;
            new(eclArray->AddrAt(m_simhitNumber)) ECLSimHit();
            eclArray[m_simhitNumber]->setCellId(cellId + 1);
            eclArray[m_simhitNumber]->setTrackId(trackID);
            eclArray[m_simhitNumber]->setPDGCode(pid);
            eclArray[m_simhitNumber]->setFlightTime(tof / ns);
            eclArray[m_simhitNumber]->setEnergyDep(edep / GeV);
            TVector3 momentum(mom.getX() / GeV, mom.getY() / GeV, mom.getZ() / GeV);
            eclArray[m_simhitNumber]->setMomentum(momentum);
            eclArray[m_simhitNumber]->setPosIn(posAve);
      */
      B2DEBUG(150, "HitNumber: " << m_simhitNumber);
      int m_simhitNumber = eclArray.getEntries() - 1;
      eclSimHitRel.add(trackID, m_simhitNumber);


      StoreArray<ECLHit> eclHitArray;
      RelationArray eclHitRel(mcParticles, eclHitArray);
      StoreObjPtr<EventMetaData> eventMetaDataPtr;
      int m_currentEvnetNumber = eventMetaDataPtr->getEvent();



      if (!eclHitArray) eclHitArray.create();


      if (firstcall == 0 || m_currentEvnetNumber != m_oldEvnetNumber) {
        m_oldEvnetNumber = m_currentEvnetNumber;
        for (int iECLCell = 0; iECLCell < 8736; iECLCell++) {
          for (int  TimeIndex = 0; TimeIndex < 80; TimeIndex++) {
            ECLHitIndex[iECLCell][TimeIndex] = -1;
          }
        }
        firstcall++;
      }


      if (m_currentEvnetNumber == m_oldEvnetNumber) {
        if ((tof / CLHEP::ns) < 8000) {
          TimeIndex = (int)((tof / CLHEP::ns) / 100);
          double E_cell = (edep / CLHEP::GeV);
          if (ECLHitIndex[cellId][TimeIndex] == -1) {

            ECLGeometryPar* eclp = ECLGeometryPar::Instance();
            PosCell =  eclp->GetCrystalPos(cellId);
            VecCell =  eclp->GetCrystalVec(cellId);
            local_pos = (15. - (posAve  - PosCell) * VecCell);
            T_ave =  6.05 + 0.0749 * local_pos - 0.00112 * local_pos * local_pos + (tof / CLHEP::ns)  ;


            //m_hitNum = eclHitArray->GetLast() + 1;
            //new(eclHitArray->AddrAt(m_hitNum)) ECLHit();
            eclHitArray.appendNew();
            m_hitNum = eclHitArray.getEntries() - 1;
            ECLHitIndex[cellId][TimeIndex] = m_hitNum;
            eclHitArray[m_hitNum]->setCellId(cellId + 1);
            eclHitArray[m_hitNum]->setEnergyDep(E_cell);
            eclHitArray[m_hitNum]->setTimeAve(T_ave);
            eclHitRel.add(trackID, m_hitNum);

          } else {
            m_hitNum = ECLHitIndex[cellId][TimeIndex];
            double old_edep = eclHitArray[m_hitNum]->getEnergyDep();
            double old_TimeAve = eclHitArray[m_hitNum]->getTimeAve();

            ECLGeometryPar* eclp = ECLGeometryPar::Instance();
            PosCell =  eclp->GetCrystalPos(cellId);
            VecCell =  eclp->GetCrystalVec(cellId);
            local_pos = (15. - (posAve  - PosCell) * VecCell);
            T_ave =  6.05 + 0.0749 * local_pos - 0.00112 * local_pos * local_pos + (tof / CLHEP::ns)  ;

            eclHitArray[m_hitNum]->setEnergyDep(old_edep + E_cell);
            eclHitArray[m_hitNum]->setTimeAve((old_edep * old_TimeAve + E_cell * T_ave) / (old_edep + E_cell));
            eclHitRel.add(trackID, m_hitNum);

          }
        }
      } else {    B2ERROR("m_currentEvnetNumber  ERROR");} //if m_oldEvnetNumber==m_oldEvnetNumber
      return (m_simhitNumber);
    }//saveSimHit



  }//namespace ecl
} //namespace Belle II

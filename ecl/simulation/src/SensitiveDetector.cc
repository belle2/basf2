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

#include <framework/logging/Logger.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <ecl/dataobjects/ECLSimHit.h>
#include <ecl/geometry/ECLGeometryPar.h>

#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>



#include <G4Step.hh>
#include <G4SteppingManager.hh>
#include <G4SDManager.hh>
#include <G4TransportationManager.hh>
#include <G4FieldManager.hh>
#include <G4MagneticField.hh>


//#include "CLHEP/Geometry/Vector3D.h"
//#include "CLHEP/Geometry/Point3D.h"

#include "TVector3.h"
#define PI 3.14159265358979323846

using namespace std;

namespace Belle2 {
  namespace ecl {

    SensitiveDetector::SensitiveDetector(G4String name, G4double thresholdEnergyDeposit, G4double thresholdKineticEnergy):
      Simulation::SensitiveDetectorBase(name, ECL), m_thresholdEnergyDeposit(thresholdEnergyDeposit),
      m_thresholdKineticEnergy(thresholdKineticEnergy), m_hitNumber(0)
    {
      StoreArray<MCParticle> mcParticles;
      StoreArray<ECLSimHit> eclSimHits;
      RelationArray eclSimHitRel(mcParticles, eclSimHits);
      registerMCParticleRelation(eclSimHitRel);
    }


    SensitiveDetector::~SensitiveDetector()
    {

    }

//    void SensitiveDetector::Initialize(G4HCofThisEvent* HCTE)
//    {
//    }

//-----------------------------------------------------
// Method invoked for every step in sensitive detector
//-----------------------------------------------------
//G4bool SensitiveDetector::ProcessHits(G4Step *aStep, G4TouchableHistory *)
    bool SensitiveDetector::step(G4Step* aStep, G4TouchableHistory*)
    {

      const G4StepPoint& preStep  = *aStep->GetPreStepPoint();
      const G4StepPoint& postStep = * aStep->GetPostStepPoint();
      G4Track& track  = *aStep->GetTrack();

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
      TVector3 position((m_startPos.getX() + m_endPos.getX()) / 2 / cm, (m_startPos.getY() + m_endPos.getY()) / 2 / cm, (m_startPos.getZ() + m_endPos.getZ()) / 2 / cm);
      m_WightedPos += position * (aStep->GetTotalEnergyDeposit());

      //Save Hit if track leaves volume or is killed
      if (track.GetNextVolume() != track.GetVolume() || track.GetTrackStatus() >= fStopAndKill) {
        int pdgCode = track.GetDefinition()->GetPDGEncoding();

        const G4VPhysicalVolume& v = * track.GetVolume();
        G4ThreeVector posCell = v.GetTranslation();
        // Get layer ID
        Mapping(v.GetName());

        if (v.GetName().find("Crystal") != string::npos) {
          int saveIndex = -999;
          double dTotalEnergy = 1 / m_energyDeposit; //avoid the error  no match for 'operator/'
          saveIndex = saveSimHit(m_cellID, m_trackID, pdgCode, m_WightedTime / m_energyDeposit , m_energyDeposit, m_momentum, m_WightedPos * dTotalEnergy);
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

      B2INFO("End Of Event");
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
      RelationArray eclSimHitRel(mcParticles, eclArray);

      m_hitNumber = eclArray->GetLast() + 1;
      new(eclArray->AddrAt(m_hitNumber)) ECLSimHit();
      eclArray[m_hitNumber]->setCellId(cellId);
      eclArray[m_hitNumber]->setTrackId(trackID);
      eclArray[m_hitNumber]->setPDGCode(pid);
      eclArray[m_hitNumber]->setFlightTime(tof / ns);
      eclArray[m_hitNumber]->setEnergyDep(edep / GeV);
      TVector3 momentum(mom.getX() / GeV, mom.getY() / GeV, mom.getZ() / GeV);
      eclArray[m_hitNumber]->setMomentum(momentum);
      eclArray[m_hitNumber]->setPosIn(posAve);
      B2DEBUG(150, "HitNumber: " << m_hitNumber);
      eclSimHitRel.add(trackID, m_hitNumber);
      return (m_hitNumber);

    }



    int SensitiveDetector::Mapping(const G4String VolumeName)
    {


      char temp1[10], temp2[10], temp3[10], temp4[30], temp5[30], temp6[10], temp7[10];

      sscanf(VolumeName.c_str(), "%[^'_']_%[^'_']_%[^'_']_%[^'_']_%[^'_']_%[^'_']_%s", temp1, temp2, temp3, temp4, temp5, temp6, temp7);

      int GSector = atoi(temp4) - 1;
      int iCry = atoi(temp6) - 1;

      if (VolumeName.c_str() == 0) {
        B2ERROR("ECL simulation cellId; Sector  " << GSector << ". Out of range.");
        return -1;
      } else if (string(VolumeName.c_str()).find("Fw") != string::npos) {

        if (iCry < 3) {
          m_thetaID = 0;
          m_phiID = GSector * 3 + iCry - 0;
          m_cellID = GSector * 3 + iCry - 0;
        } else if (iCry < 6) {
          m_thetaID = 1;
          m_phiID = GSector * 3 + (iCry - 3) ;
          m_cellID = GSector * 3 + (iCry - 3) + 16 * 3;
        } else if (iCry < 10) {
          m_thetaID = 2;
          m_phiID = GSector * 4 + (iCry - 6) ;
          m_cellID = GSector * 4 + (iCry - 6) + 16 * 6;
        } else if (iCry < 14) {
          m_thetaID = 3;
          m_phiID = GSector * 4 + (iCry - 10) ;
          m_cellID = GSector * 4 + (iCry - 10) + 16 * 10;
        } else if (iCry < 18) {
          m_thetaID = 4;
          m_phiID = GSector * 4 + (iCry - 14) ;
          m_cellID = GSector * 4 + (iCry - 14) + 16 * 14;
        } else if (iCry < 24) {
          m_thetaID = 5;
          m_phiID = GSector * 6 + (iCry - 18) ;
          m_cellID = GSector * 6 + (iCry - 18) + 16 * 18;
        } else if (iCry < 30) {
          m_thetaID = 6;
          m_phiID = GSector * 6 + (iCry - 24) ;
          m_cellID = GSector * 6 + (iCry - 24) + 16 * 24;
        } else if (iCry < 36) {
          m_thetaID = 7;
          m_phiID = GSector * 6 + (iCry - 30) ;
          m_cellID = GSector * 6 + (iCry - 30) + 16 * 30;
        } else if (iCry < 42) {
          m_thetaID = 8;
          m_phiID = GSector * 6 + (iCry - 36) ;
          m_cellID = GSector * 6 + (iCry - 36) + 16 * 36;
        } else if (iCry < 48) {
          m_thetaID = 9;
          m_phiID = GSector * 6 + (iCry - 42) ;
          m_cellID = GSector * 6 + (iCry - 42) + 16 * 42;
        } else if (iCry < 54) {
          m_thetaID = 10;
          m_phiID = GSector * 6 + (iCry - 48) ;
          m_cellID = GSector * 6 + (iCry - 48) + 16 * 48;
        } else if (iCry < 63) {
          m_thetaID = 11;
          m_phiID = GSector * 9 + (iCry - 54) ;
          m_cellID = GSector * 9 + (iCry - 54) + 16 * 54;
        } else if (iCry < 72) {
          m_thetaID = 12;
          m_phiID = GSector * 9 + (iCry - 63) ;
          m_cellID = GSector * 9 + (iCry - 63) + 16 * 63;

        }
      } else if (string(VolumeName.c_str()).find("Br") != string::npos) {
        m_phiID = GSector;
        m_thetaID = iCry + 13 ;
        if (m_phiID == -1) m_phiID = 143;
        m_cellID = 1152 + (m_thetaID - 13) * 144 + m_phiID;

      } else {

        iCry = iCry - 72;
        if (iCry < 9) {
          m_thetaID = 59;
          m_phiID = (GSector) * 9 + iCry - 0;
          m_cellID = (GSector) * 9 + iCry - 0 + 7776;
        } else if (iCry < 18) {
          m_thetaID = 60;
          m_phiID = (GSector) * 9 + (iCry - 9) ;
          m_cellID = (GSector) * 9 + (iCry - 9) + 16 * 9 + 7776;
        } else if (iCry < 24) {
          m_thetaID = 61;
          m_phiID = (GSector) * 6 + (iCry - 18) ;
          m_cellID = (GSector) * 6 + (iCry - 18) + 16 * 18 + 7776;
        } else if (iCry < 30) {
          m_thetaID = 62;
          m_phiID = (GSector) * 6 + (iCry - 24) ;
          m_cellID = (GSector) * 6 + (iCry - 24) + 16 * 24 + 7776;
        } else if (iCry < 36) {
          m_thetaID = 63;
          m_phiID = (GSector) * 6 + (iCry - 30) ;
          m_cellID = (GSector) * 6 + (iCry - 30) + 16 * 30 + 7776;
        } else if (iCry < 42) {
          m_thetaID = 64;
          m_phiID = (GSector) * 6 + (iCry - 36) ;
          m_cellID = (GSector) * 6 + (iCry - 36) + 16 * 36 + 7776;
        } else if (iCry < 48) {
          m_thetaID = 65;
          m_phiID = (GSector) * 6 + (iCry - 42) ;
          m_cellID = (GSector) * 6 + (iCry - 42) + 16 * 42 + 7776;
        } else if (iCry < 52) {
          m_thetaID = 66;
          m_phiID = (GSector) * 4 + (iCry - 48) ;
          m_cellID = (GSector) * 4 + (iCry - 48) + 16 * 48 + 7776;
        } else if (iCry < 56) {
          m_thetaID = 67;
          m_phiID = (GSector) * 4 + (iCry - 52) ;
          m_cellID = (GSector) * 4 + (iCry - 52) + 16 * 52 + 7776;
        } else if (iCry < 60) {
          m_thetaID = 68;
          m_phiID = (GSector) * 4 + (iCry - 56) ;
          m_cellID = (GSector) * 4 + (iCry - 56) + 16 * 56 + 7776;
        }
      }

      if (m_cellID > 1151 && string(VolumeName.c_str()).find("FW") != string::npos) {
        B2ERROR("SensitiveDetector: Cellid ERR FW GSector: " << GSector << " iCry: " << iCry << " " << string(VolumeName.c_str()));
      } else if (m_cellID < 7776 && string(VolumeName.c_str()).find("Bw") != string::npos) {
        B2ERROR("SensitiveDetector: Cellid ERR BW GSector: " << GSector << " iCry: " << iCry << " " << string(VolumeName.c_str()));
      } else if ((m_cellID > 7775 || m_cellID < 1152) && string(VolumeName.c_str()).find("Br") != string::npos) {
        B2ERROR("SensitiveDetector: Cellid ERR Br GSector: " << GSector << " iCry: " << iCry << " " << string(VolumeName.c_str()));
      }
      return m_cellID;

    }
  }//namespace ecl
} //namespace Belle II

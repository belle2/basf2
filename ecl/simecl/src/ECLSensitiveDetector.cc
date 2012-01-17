/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:                                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/



#include <ecl/simecl/ECLSensitiveDetector.h>

#include <framework/logging/Logger.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <ecl/hitecl/ECLSimHit.h>
#include <ecl/hitecl/ECLEBSimHit.h>
#include <ecl/geoecl/ECLGeometryPar.h>

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


  ECLSensitiveDetector::ECLSensitiveDetector(G4String name, G4double thresholdEnergyDeposit, G4double thresholdKineticEnergy):
    Simulation::SensitiveDetectorBase(name, ECL), m_thresholdEnergyDeposit(thresholdEnergyDeposit),
    m_thresholdKineticEnergy(thresholdKineticEnergy), m_hitNumber(0), m_EBhitNumber(0)
  {
    StoreArray<MCParticle> mcParticles;
    StoreArray<ECLSimHit> eclSimHits;
    StoreArray<ECLEBSimHit> eclEBSimHits;
    RelationArray eclSimHitRel(mcParticles, eclSimHits);
    registerMCParticleRelation(eclSimHitRel);
  }


  ECLSensitiveDetector::~ECLSensitiveDetector()
  {

  }
  void ECLSensitiveDetector::Initialize(G4HCofThisEvent* HCTE)
  {
    // Create a new hit collection

    // Assign a unique ID to the hits collection

    // Attach collections to the hits collection of this event

    // Initialize
    B2INFO("SensitiveDetector ECL initialized");
  }


//-----------------------------------------------------
// Method invoked for every step in sensitive detector
//-----------------------------------------------------
//G4bool ECLSensitiveDetector::ProcessHits(G4Step *aStep, G4TouchableHistory *)
  bool ECLSensitiveDetector::step(G4Step* aStep, G4TouchableHistory*)
  {

    // Get deposited energy
    const G4double edep = aStep->GetTotalEnergyDeposit();

    // Get step length
    const G4double stepLength = aStep->GetStepLength();
    if (stepLength == 0.) return false;


    G4int FirstStepFlag = 0;
    G4Track& t = * aStep->GetTrack();
    const G4double tof = t.GetGlobalTime();
    if (isnan(tof)) {
      B2ERROR("ECLSensitiveDetector: global time is nan");
      return false;
    }

    const G4int pid = t.GetDefinition()->GetPDGEncoding();
    const G4int trackID = t.GetTrackID();

    const G4VPhysicalVolume& v = * t.GetVolume();
    const G4StepPoint& in = * aStep->GetPreStepPoint();
    const G4StepPoint& out = * aStep->GetPostStepPoint();
    const G4ThreeVector& posIn = in.GetPosition();
    const G4ThreeVector& posOut = out.GetPosition();
    const G4ThreeVector momIn(in.GetMomentum().x(), in.GetMomentum().y(),
                              in.GetMomentum().z());

    const G4ThreeVector posCell = v.GetTranslation();
    // Get layer ID
    Mapping(v.GetName());
    ECLGeometryPar* eclp = ECLGeometryPar::Instance();
    TVector3 VecCell =  eclp->GetCrystalVec(m_cellID);
    TVector3 Pin(momIn.getX(), momIn.getY(), momIn.getZ());

    //cout << v.GetName() << " CellID " << m_cellID << " " << v.GetCopyNo() << endl;


    if (v.GetName().find("Crystal") != string::npos) {
      int saveIndex = -999;
      saveIndex = saveSimHit(m_cellID, m_thetaID, m_phiID  , trackID, pid, tof, edep, FirstStepFlag, momIn, posCell, posIn, posOut);
    }

    if (v.GetName().find("Diode") != string::npos) {
      int saveEBIndex = -999;
      if (trackID != oldtrack && m_cellID != oldcellId) {
        oldtrack = trackID; oldcellId = m_cellID;
        FirstStepFlag = 1;
      }
      saveEBIndex = saveEBSimHit(m_cellID, m_thetaID, m_phiID, trackID, pid, tof, edep, FirstStepFlag, momIn, posCell, posIn, posOut);
    }

    // Ge layer ID
//  const unsigned layerId = v.GetCopyNo();
    return true;
  }


  void ECLSensitiveDetector::EndOfEvent(G4HCofThisEvent*)
  {

    B2INFO("End Of Event");
  }

  int ECLSensitiveDetector::saveEBSimHit(
    const G4int cellId,
    const G4int thetaId,
    const G4int phiId,
    const G4int trackID,
    const G4int pid,
    const G4double lof,
    const G4double edep,
    const G4double FirstStep,
    const G4ThreeVector& mom,
    const G4ThreeVector& posCell,
    const G4ThreeVector& posIn,
    const G4ThreeVector& posOut)
  {
    //change Later
    StoreArray<ECLEBSimHit> eclEBArray;
    m_EBhitNumber = eclEBArray->GetLast() + 1;
    new(eclEBArray->AddrAt(m_EBhitNumber)) ECLEBSimHit();


    eclEBArray[m_EBhitNumber]->setThetaId(thetaId);
    eclEBArray[m_EBhitNumber]->setPhiId(phiId);
    eclEBArray[m_EBhitNumber]->setCellId(cellId);
    eclEBArray[m_EBhitNumber]->setTrackId(trackID);
    eclEBArray[m_EBhitNumber]->setPDGCode(pid);
    eclEBArray[m_EBhitNumber]->setFlightLength(lof);
    eclEBArray[m_EBhitNumber]->setEnergyDep(edep / GeV);
    eclEBArray[m_EBhitNumber]->setStepLength(FirstStep);
    TVector3 momentum(mom.getX() / GeV, mom.getY() / GeV, mom.getZ() / GeV);
    eclEBArray[m_EBhitNumber]->setMomentum(momentum);
    TVector3 posCellv(posCell.getX() / cm, posCell.getY() / cm, posCell.getZ() / cm);
    eclEBArray[m_EBhitNumber]->setPosCell(posCellv);
    TVector3 positionIn(posIn.getX() / cm, posIn.getY() / cm, posIn.getZ() / cm);
    eclEBArray[m_EBhitNumber]->setPosIn(positionIn);
    TVector3 positionOut(posOut.getX() / cm, posOut.getY() / cm, posOut.getZ() / cm);
    eclEBArray[m_EBhitNumber]->setPosOut(positionOut);
    return (m_EBhitNumber);
  }


  int ECLSensitiveDetector::saveSimHit(
    const G4int cellId,
    const G4int thetaId,
    const G4int phiId,
    const G4int trackID,
    const G4int pid,
    const G4double tof,
    const G4double edep,
    const G4double FirstStep,
    const G4ThreeVector& mom,
    const G4ThreeVector& posCell,
    const G4ThreeVector& posIn,
    const G4ThreeVector& posOut)
  {
    StoreArray<MCParticle> mcParticles;
    //change Later
    StoreArray<ECLSimHit> eclArray;
    RelationArray eclSimHitRel(mcParticles, eclArray);

    m_hitNumber = eclArray->GetLast() + 1;
    new(eclArray->AddrAt(m_hitNumber)) ECLSimHit();
    eclArray[m_hitNumber]->setThetaId(thetaId);
    eclArray[m_hitNumber]->setPhiId(phiId);
    eclArray[m_hitNumber]->setCellId(cellId);
    eclArray[m_hitNumber]->setTrackId(trackID);
    eclArray[m_hitNumber]->setPDGCode(pid);
    eclArray[m_hitNumber]->setFlightTime(tof / ns);
    eclArray[m_hitNumber]->setEnergyDep(edep / GeV);
    eclArray[m_hitNumber]->setStepLength(FirstStep);
    TVector3 momentum(mom.getX() / GeV, mom.getY() / GeV, mom.getZ() / GeV);
    eclArray[m_hitNumber]->setMomentum(momentum);
    TVector3 posCellv(posCell.getX() / cm, posCell.getY() / cm, posCell.getZ() / cm);
    eclArray[m_hitNumber]->setPosCell(posCellv);
    TVector3 positionIn(posIn.getX() / cm, posIn.getY() / cm, posIn.getZ() / cm);
    eclArray[m_hitNumber]->setPosIn(positionIn);
    TVector3 positionOut(posOut.getX() / cm, posOut.getY() / cm, posOut.getZ() / cm);
    eclArray[m_hitNumber]->setPosOut(positionOut);
    B2DEBUG(150, "HitNumber: " << m_hitNumber);
    eclSimHitRel.add(trackID, m_hitNumber);
    return (m_hitNumber);

  }



  int ECLSensitiveDetector::Mapping(const G4String VolumeName)
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
      B2ERROR("ECLSensitiveDetector: Cellid ERR FW GSector: " << GSector << " iCry: " << iCry << " " << string(VolumeName.c_str()));
    } else if (m_cellID < 7776 && string(VolumeName.c_str()).find("Bw") != string::npos) {
      B2ERROR("ECLSensitiveDetector: Cellid ERR BW GSector: " << GSector << " iCry: " << iCry << " " << string(VolumeName.c_str()));
    } else if ((m_cellID > 7775 || m_cellID < 1152) && string(VolumeName.c_str()).find("Br") != string::npos) {
      B2ERROR("ECLSensitiveDetector: Cellid ERR Br GSector: " << GSector << " iCry: " << iCry << " " << string(VolumeName.c_str()));
    }
    return m_cellID;

  }

} //namespace Belle II

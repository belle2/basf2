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
#include <framework/logging/Logger.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/dataobjects/Relation.h>
//#include <ecl/hitecl/include/ECLSimHit.h>
#include <ecl/hitecl/ECLSimHit.h>

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


#include "CLHEP/Geometry/Vector3D.h"
#include "CLHEP/Geometry/Point3D.h"

#include "TVector3.h"
#define PI 3.14159265358979323846

using namespace Belle2;

ECLSensitiveDetector::ECLSensitiveDetector(G4String name) : SensitiveDetectorBase(name)
{
  addRelationCollection(DEFAULT_MCPART_TO_ECLSIMHITS);

}

ECLSensitiveDetector::~ECLSensitiveDetector()
{

}

void ECLSensitiveDetector::Initialize(G4HCofThisEvent * HCTE)
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
G4bool ECLSensitiveDetector::ProcessHits(G4Step *aStep, G4TouchableHistory *)
{

//http://geant4.web.cern.ch/geant4/UserDocumentation/UsersGuides/ForApplicationDeveloper/html/ch05.html

  const G4double edep = aStep->GetTotalEnergyDeposit();
  if (edep == 0.) return false;

  // Get step length
  const G4double stepLength = aStep->GetStepLength();
  const G4double s_in_layer = stepLength / cm;
//  if (stepLength == 0.) return false;

  // Get step information
  const G4Track & t = * aStep->GetTrack();

  const G4double tof = t.GetGlobalTime();
  if (isnan(tof)) {
    B2ERROR("ECLSensitiveDetector: global time is nan");
    return false;
  }

  const G4int pid = t.GetDefinition()->GetPDGEncoding();
  const G4int trackID = t.GetTrackID();

  const G4VPhysicalVolume & v = * t.GetVolume();
  const G4StepPoint & in = * aStep->GetPreStepPoint();
  const G4StepPoint & out = * aStep->GetPostStepPoint();
  const G4ThreeVector & posIn = in.GetPosition();
  const G4ThreeVector & posOut = out.GetPosition();
  const G4ThreeVector momIn(in.GetMomentum().x(), in.GetMomentum().y(),
                            in.GetMomentum().z());

  const G4ThreeVector posCell = v.GetTranslation();
  // Get layer ID

  Mapping(v.GetName());

  int saveIndex = -999;
  saveIndex = saveSimHit(m_cellID, m_thetaID, m_phiID  , trackID, pid, tof, edep, s_in_layer * cm, momIn, posCell, posIn, posOut);

  //Set the SeenInDetector flag
  setSeenInDetectorFlag(aStep, MCParticle::c_LastSeenInECL);

  //Add relation between the MCParticle and the hit.
  //The index of the MCParticle has to be set to the TrackID and will be
  //replaced later by the correct MCParticle index automatically.

  StoreArray<Relation> mcPartToSimHits(getRelationCollectionName());
  StoreArray<MCParticle> mcPartArray(DEFAULT_MCPARTICLES);
  if (saveIndex < 0) {B2FATAL("SimHit wasn't saved despite charge != 0");}
  StoreArray<ECLSimHit> eclArray(DEFAULT_ECLSIMHITS);

  new(mcPartToSimHits->AddrAt(saveIndex)) Relation(mcPartArray, eclArray, trackID, saveIndex);

  return true;
}


void ECLSensitiveDetector::EndOfEvent(G4HCofThisEvent *)
{

  B2INFO("End Of Event");
}


int ECLSensitiveDetector::saveSimHit(
  const G4int cellId,
  const G4int thetaId,
  const G4int phiId,
  const G4int trackID,
  const G4int pid,
  const G4double tof,
  const G4double edep,
  const G4double stepLength,
  const G4ThreeVector & mom,
  const G4ThreeVector & posCell,
  const G4ThreeVector & posIn,
  const G4ThreeVector & posOut)
{
  //change Later
  StoreArray<ECLSimHit> eclArray(DEFAULT_ECLSIMHITS);
  m_hitNumber = eclArray->GetLast() + 1;
  new(eclArray->AddrAt(m_hitNumber)) ECLSimHit();
  eclArray[m_hitNumber]->setThetaId(thetaId);
  eclArray[m_hitNumber]->setPhiId(phiId);
  eclArray[m_hitNumber]->setCellId(cellId);
  eclArray[m_hitNumber]->setTrackId(trackID);
  eclArray[m_hitNumber]->setPDGCode(pid);
  eclArray[m_hitNumber]->setFlightTime(tof / ns);
  eclArray[m_hitNumber]->setEnergyDep(edep / GeV);
  eclArray[m_hitNumber]->setStepLength(stepLength / cm);
  TVector3 momentum(mom.getX() / GeV, mom.getY() / GeV, mom.getZ() / GeV);
  eclArray[m_hitNumber]->setMomentum(momentum);
  TVector3 posCellv(posCell.getX() / cm, posCell.getY() / cm, posCell.getZ() / cm);
  eclArray[m_hitNumber]->setPosCell(posCellv);
  TVector3 positionIn(posIn.getX() / cm, posIn.getY() / cm, posIn.getZ() / cm);
  eclArray[m_hitNumber]->setPosIn(positionIn);
  TVector3 positionOut(posOut.getX() / cm, posOut.getY() / cm, posOut.getZ() / cm);
  eclArray[m_hitNumber]->setPosOut(positionOut);
//  B2INFO("SensitiveDetector ECL initialized");
  B2INFO("HitNumber: " << m_hitNumber);
  return (m_hitNumber);
}


int ECLSensitiveDetector::Mapping(const G4String VolumeName)
{

  char tmp1[10], tmp2[10], EclSector[10], CrystalID[10];
  sscanf(VolumeName.c_str(), "%[^'_']_%[^'_']_%[^'_']_%s", tmp1, tmp2, EclSector, CrystalID);
  int GSector = atoi(EclSector);
  int iCry = atoi(CrystalID);

  if (!(GSector<104 && GSector> -1)) {
    B2ERROR("ECL simulation cellId; Sector  " << GSector << ". Out of range.");
    return -1;
  } else if (GSector < 16) {

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
  } else if (GSector < 88) {
    m_phiID = (GSector - 16) * 2 + (iCry  % 2) - 1;
    m_thetaID = iCry / 2 + 13 ;
    if (m_phiID == -1) m_phiID = 143;
    m_cellID = 1152 + (m_thetaID - 13) * 144 + m_phiID;

  } else {
    if (iCry < 9) {
      m_thetaID = 59;
      m_phiID = (GSector - 88) * 9 + iCry - 0;
      m_cellID = (GSector - 88) * 9 + iCry - 0;
    } else if (iCry < 18) {
      m_thetaID = 60;
      m_phiID = (GSector - 88) * 9 + (iCry - 9) ;
      m_cellID = (GSector - 88) * 9 + (iCry - 9) + 16 * 9 + 7776;
    } else if (iCry < 24) {
      m_thetaID = 61;
      m_phiID = (GSector - 88) * 6 + (iCry - 18) ;
      m_cellID = (GSector - 88) * 6 + (iCry - 18) + 16 * 18 + 7776;
    } else if (iCry < 30) {
      m_thetaID = 62;
      m_phiID = (GSector - 88) * 6 + (iCry - 24) ;
      m_cellID = (GSector - 88) * 6 + (iCry - 24) + 16 * 24 + 7776;
    } else if (iCry < 36) {
      m_thetaID = 63;
      m_phiID = (GSector - 88) * 6 + (iCry - 30) ;
      m_cellID = (GSector - 88) * 6 + (iCry - 30) + 16 * 30 + 7776;
    } else if (iCry < 42) {
      m_thetaID = 64;
      m_phiID = (GSector - 88) * 6 + (iCry - 36) ;
      m_cellID = (GSector - 88) * 6 + (iCry - 36) + 16 * 36 + 7776;
    } else if (iCry < 48) {
      m_thetaID = 65;
      m_phiID = (GSector - 88) * 6 + (iCry - 42) ;
      m_cellID = (GSector - 88) * 6 + (iCry - 42) + 16 * 42 + 7776;
    } else if (iCry < 52) {
      m_thetaID = 66;
      m_phiID = (GSector - 88) * 4 + (iCry - 48) ;
      m_cellID = (GSector - 88) * 4 + (iCry - 48) + 16 * 48 + 7776;
    } else if (iCry < 56) {
      m_thetaID = 67;
      m_phiID = (GSector - 88) * 4 + (iCry - 52) ;
      m_cellID = (GSector - 88) * 4 + (iCry - 52) + 16 * 52 + 7776;
    } else if (iCry < 60) {
      m_thetaID = 68;
      m_phiID = (GSector - 88) * 4 + (iCry - 56) ;
      m_cellID = (GSector - 88) * 4 + (iCry - 56) + 16 * 56 + 7776;
    }
  }
  return m_cellID;

}


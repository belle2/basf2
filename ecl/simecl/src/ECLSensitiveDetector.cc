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
//#include <framework/dataobjects/Relation.h>
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

ECLSensitiveDetector::ECLSensitiveDetector(G4String name, G4double thresholdEnergyDeposit, G4double thresholdKineticEnergy):
    SensitiveDetectorBase(name, ECL), m_thresholdEnergyDeposit(thresholdEnergyDeposit),
    m_thresholdKineticEnergy(thresholdKineticEnergy), m_hitNumber(0), m_EBhitNumber(0)
{
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
//G4bool ECLSensitiveDetector::ProcessHits(G4Step *aStep, G4TouchableHistory *)
bool ECLSensitiveDetector::step(G4Step *aStep, G4TouchableHistory *)
{

  return true;
}


void ECLSensitiveDetector::EndOfEvent(G4HCofThisEvent *)
{

  B2INFO("End Of Event");
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
      m_cellID = (GSector - 88) * 9 + iCry - 0 + 7776;
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


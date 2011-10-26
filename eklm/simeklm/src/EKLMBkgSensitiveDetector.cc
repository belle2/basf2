/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Timofey Uglov                                             *
 * based on class by  Luka Santelj and Marko Petric                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <eklm/eklmhit/EKLMBackHit.h>
#include <eklm/simeklm/EKLMBkgSensitiveDetector.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>

#include <eklm/geoeklm/G4PVPlacementGT.h>


#include <G4Track.hh>
#include <G4Step.hh>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <TVector3.h>

using namespace std;

namespace Belle2 {


  EKLMBkgSensitiveDetector::EKLMBkgSensitiveDetector(const char* subDett, int iden, int hitType):
      Simulation::SensitiveDetectorBase("BKG", EKLMBkgSensitiveDetector::KLM)
  {
    StoreArray<MCParticle> mcParticles;
    StoreArray<EKLMBackHit> backHits;
    RelationArray  backHitRel(mcParticles, backHits);
    registerMCParticleRelation(backHitRel);

    std::string subDet = subDett;
    if (subDet == "IR")    m_subDet = 0;
    else if (subDet == "PXD")   m_subDet = 1;
    else if (subDet == "SVD")   m_subDet = 2;
    else if (subDet == "CDC")   m_subDet = 3;
    else if (subDet == "ARICH")   m_subDet = 4;
    else if (subDet == "TOP") m_subDet = 5;
    else if (subDet == "ECL")   m_subDet = 6;
    else if (subDet == "EKLM")  m_subDet = 7;
    else if (subDet == "BKLM")  m_subDet = 8;
    else                        m_subDet = 99;

    m_hitType = hitType;
    m_identifier = iden;

  }


  G4bool EKLMBkgSensitiveDetector::step(G4Step* aStep, G4TouchableHistory*)
  {

    const G4StepPoint& preStep  = *aStep->GetPreStepPoint();


    //if (preStep.GetStepStatus() != fGeomBoundary) return false;

    //Get particle ID
    G4Track& track  = *aStep->GetTrack();
    const G4int pdgCode = track.GetDefinition()->GetPDGEncoding();

//     switch (m_hitType) {
//       case 0: if (pdgCode != 22 && abs(pdgCode) != 2112) return false; break;
//       case 1: if (abs(pdgCode) != 2112)                  return false; break;
//       case 2: if (pdgCode != 22)                         return false; break;
//     }

    //Get time (check for proper global time)
    const G4double globalTime = track.GetGlobalTime();
    //Get step information

    //Get world position
    const G4ThreeVector& worldPosition = preStep.GetPosition();
    //Get momentum
    const G4ThreeVector& momentum = preStep.GetMomentum() ;
    //Get energy
    const G4double energy = track.GetKineticEnergy() * Unit::MeV;

    // -----------  EKLM-specific information
    const G4double energyDeposit = aStep->GetTotalEnergyDeposit() * Unit::MeV;
    const G4int    trackID = track.GetTrackID();
    const G4int    ParentTrackID = track.GetParentID();
    const bool isFirstStep = (preStep.GetStepStatus() != fGeomBoundary);

    G4PVPlacementGT * pvgt = (G4PVPlacementGT*)(aStep->GetPreStepPoint()->GetPhysicalVolume());
    const std::string pvName = pvgt->GetName();


    //------------------------------------------------------------
    //                Create EKLMBackHit and save it to datastore
    //------------------------------------------------------------

    TVector3 pos(worldPosition.x() / cm, worldPosition.y() / cm, worldPosition.z() / cm);
    TVector3 mom(momentum.x() * Unit::MeV, momentum.y() * Unit::MeV , momentum.z() * Unit::MeV);
    StoreArray<EKLMBackHit> backHits;
    int nentr = backHits->GetLast() + 1;
    new(backHits->AddrAt(nentr)) EKLMBackHit(m_subDet, m_identifier, pdgCode, globalTime, energy, pos, mom, energyDeposit, trackID, ParentTrackID, isFirstStep, pvName);

    return true;
  }


} // end of namespace Belle2

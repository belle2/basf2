/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov, Kirill Chilikin                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#include <eklm/simeklm/EKLMSensitiveDetector.h>

#include <framework/logging/Logger.h>

#include <eklm/eklmutils/EKLMutils.h>
#include <eklm/geoeklm/G4PVPlacementGT.h>

#include "G4Step.hh"
#include "G4SteppingManager.hh"
#include "G4SDManager.hh"
#include "G4TransportationManager.hh"
#include "G4FieldManager.hh"
#include "G4MagneticField.hh"

#include <generators/dataobjects/MCParticle.h>
#include <framework/datastore/RelationArray.h>



namespace Belle2 {


  EKLMSensitiveDetector::EKLMSensitiveDetector(G4String name,
                                               G4double thresholdEnergyDeposit,
                                               G4double thresholdKineticEnergy)
      : Simulation::SensitiveDetectorBase(name, KLM)
  {
    m_mode = -1;
  }

  void EKLMSensitiveDetector::Initialize(G4HCofThisEvent * HCTE)
  {
    m_HitNumber = 0;
  }

  int EKLMSensitiveDetector::getMode(G4Step *aStep)
  {
    G4PVPlacementGT *pvgt;
    if (m_mode >= 0)
      return m_mode;
    pvgt = (G4PVPlacementGT*)(aStep->GetPreStepPoint()->GetPhysicalVolume());
    m_mode = pvgt->getMode();
    return m_mode;
  }

  //-----------------------------------------------------
  // Method invoked for every step in sensitive detector
  //-----------------------------------------------------
  bool EKLMSensitiveDetector::step(G4Step *aStep, G4TouchableHistory *)
  {
    // check if it is first step in volume
    bool isFirstStep(aStep->GetPreStepPoint()->GetStepStatus() == fGeomBoundary);


    // Get deposited energy
    const G4double eDep = aStep->GetTotalEnergyDeposit();

    // ignore tracks with small energy deposition
    double eDepositionThreshold = 0.001 / MeV;  // should be accessible via xml
    if (eDep <= eDepositionThreshold) return false;


    const G4Track & track = * aStep->GetTrack();

    // Get tracks charge
    //    const G4double charge = track.GetDefinition()->GetPDGCharge();

    // ignore neutrals in EKLM
    //    if (charge == 0.) return false;

    // get time of hit
    const G4double hitTime = track.GetGlobalTime();

    G4VPhysicalVolume *pv = aStep->GetPreStepPoint()->GetPhysicalVolume();

    /*
     * drop hit if global time is nan or if it is  mothe than
     * hitTimeThreshold (to avoid nuclei fission signals)
     */
    if (isnan(hitTime)) {
      B2ERROR("EKLMSensitiveDetector: global time is nan");
      return false;

      double hitTimeThreshold = 500; // should be parameter or accessev via xml
      if (hitTime > hitTimeThreshold) {
        B2INFO("EKLMSensitiveDetector:  "
               "ALL HITS WITH TIME > hitTimeThreshold ARE DROPPED!!!!!!!");
        return false;
      }

    }


    // Get particle information
    const G4int PDGcode = track.GetDefinition()->GetPDGEncoding();


    // no conversion btw. G4ThreeVector and TVector3. Sad but true
    const G4ThreeVector & gpos = 0.5 *
                                 (aStep->GetPostStepPoint()->GetPosition() +
                                  aStep->GetPreStepPoint()->GetPosition());

    const TVector3 & gposRoot = TVector3(gpos.z(), gpos.y(), gpos.z());

    const G4ThreeVector & lpos = aStep->GetPreStepPoint()->
                                 GetTouchableHandle()->GetHistory()->
                                 GetTopTransform().TransformPoint(gpos);

    const TVector3 & lposRoot = TVector3(lpos.z(), lpos.y(), lpos.z());

    //creates hit
    StoreArray<EKLMSimHit> simHitsArray;
    EKLMSimHit *hit = new(simHitsArray->AddrAt(simHitsArray.getEntries()))
    EKLMSimHit(pv, gposRoot, lposRoot, hitTime, PDGcode,  eDep);
    if (hit == NULL) {
      B2ERROR("Memory allocation error.");
      return false;
    }
    G4PVPlacementGT *pvgt = (G4PVPlacementGT*)pv;
    pvgt = pvgt->getMother();
    pvgt = pvgt->getMother();
    hit->set_nStrip(pvgt->getID());
    pvgt = pvgt->getMother();
    hit->set_nPlane(pvgt->getID());
    pvgt = pvgt->getMother();
    hit->set_nSector(pvgt->getID());
    pvgt = pvgt->getMother();
    hit->set_nLayer(pvgt->getID());
    pvgt = pvgt->getMother();
    hit->set_nEndcap(pvgt->getID());

    hit->setFirstHit(isFirstStep);
    hit->setTrackID(track.GetTrackID());
    hit->setParentTrackID(track.GetParentID());


    StoreArray<MCParticle> MCParticlesArray;
    RelationArray particleToSimHitsRelation(MCParticlesArray, simHitsArray);
    registerMCParticleRelation(particleToSimHitsRelation);
    particleToSimHitsRelation.add(track.GetTrackID(),
                                  simHitsArray.getEntries());

    return true;
  }

  void EKLMSensitiveDetector::EndOfEvent(G4HCofThisEvent *)
  {
  }

} //namespace Belle II


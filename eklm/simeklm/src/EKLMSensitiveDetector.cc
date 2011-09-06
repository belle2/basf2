/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#include <eklm/simeklm/EKLMSensitiveDetector.h>

#include <framework/logging/Logger.h>


#include <eklm/eklmutils/EKLMutils.h>
//#include <framework/datastore/Units.h>

#include "G4Step.hh"
#include "G4SteppingManager.hh"
#include "G4SDManager.hh"
#include "G4TransportationManager.hh"
#include "G4FieldManager.hh"
#include "G4MagneticField.hh"



namespace Belle2 {


  EKLMSensitiveDetector::EKLMSensitiveDetector(G4String name,
                                               G4double thresholdEnergyDeposit, G4double thresholdKineticEnergy)
      : Simulation::SensitiveDetectorBase(name, KLM)
  {
  }

  void EKLMSensitiveDetector::Initialize(G4HCofThisEvent * HCTE)
  {
  }

  //-----------------------------------------------------
  // Method invoked for every step in sensitive detector
  //-----------------------------------------------------
  bool EKLMSensitiveDetector::step(G4Step *aStep, G4TouchableHistory *)
  {
    // Get deposited energy
    const G4double eDep = aStep->GetTotalEnergyDeposit();

    // ignore tracks with small energy deposition
    double eDepositionThreshold = 0.001 / MeV;  // should be accessible via xml
    if (eDep <= eDepositionThreshold) return false;


    const G4Track & track = * aStep->GetTrack();

    // Get tracks charge
    const G4double charge = track.GetDefinition()->GetPDGCharge();

    // ignore neutrals in EKLM
    if (charge == 0.) return false;

    // get time of hit
    const G4double hitTime = track.GetGlobalTime();

    G4VPhysicalVolume *pv = aStep->GetPostStepPoint()->GetPhysicalVolume();

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

    const G4ThreeVector & gpos = 0.5 *
                                 (aStep->GetPostStepPoint()->GetPosition() +
                                  aStep->GetPreStepPoint()->GetPosition()) * 0.1; // to convert to cm

    const G4ThreeVector & lpos = aStep->GetPostStepPoint()->
                                 GetTouchableHandle()->GetHistory()->
                                 GetTopTransform().TransformPoint(gpos);

    //creates hit
    EKLMSimHit *hit = new  EKLMSimHit(pv, gpos, lpos, hitTime, PDGcode,  eDep);

    // store hit
    storeEKLMObject("SimHitsEKLMArray", hit);


    return true;
  }

  void EKLMSensitiveDetector::EndOfEvent(G4HCofThisEvent *)
  {
  }

} //namespace Belle II


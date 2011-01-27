/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#include <eklm/simeklm/EKLMSensetiveDetector.h>
#include <eklm/simeklm/EKLMDigitizer.h>
#include <framework/logging/Logger.h>


#include <eklm/eklmutils/EKLMutils.h>
#include <framework/datastore/Units.h>

#include "G4Step.hh"
#include "G4SteppingManager.hh"
#include "G4SDManager.hh"
#include "G4TransportationManager.hh"
#include "G4FieldManager.hh"
#include "G4MagneticField.hh"



namespace Belle2 {


  EKLMSensetiveDetector::EKLMSensetiveDetector(G4String name, G4double thresholdEnergyDeposit, G4double thresholdKineticEnergy):
      SensitiveDetectorBase(name), m_ThresholdEnergyDeposit(thresholdEnergyDeposit),
      m_ThresholdKineticEnergy(thresholdKineticEnergy), m_HitCollection(0),
      m_HCID(-1)
  {

    G4String CollName1 = name + "_Collection";
    collectionName.insert(CollName1);
  }

  void EKLMSensetiveDetector::Initialize(G4HCofThisEvent * HCTE)
  {
    // Create a new hit collection
    m_HitCollection = new EKLMSimHitsCollection(SensitiveDetectorName, collectionName[0]);

    // Assign a unique ID to the hits collection
    if (m_HCID < 0) {
      m_HCID = G4SDManager::GetSDMpointer()->GetCollectionID(m_HitCollection);
    }

    // Attach collections to HitsCollectionsOfThisEvent
    HCTE -> AddHitsCollection(m_HCID, m_HitCollection);

  }

//-----------------------------------------------------
// Method invoked for every step in sensitive detector
//-----------------------------------------------------
  G4bool EKLMSensetiveDetector::ProcessHits(G4Step *aStep, G4TouchableHistory *)
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

    // drop hit if global time is nan or if it is  mothe than hitTimeThreshold (to avoid nuclei fission signals)
    if (isnan(hitTime)) {
      B2ERROR("EKLMSensetiveDetector: global time is nan");
      return false;

      double hitTimeThreshold = 500; // should be parameter or accessev via xml
      if (hitTime > hitTimeThreshold) {
        B2INFO("EKLMSensetiveDetector:  ALL HITS WITH TIME > hitTimeThreshold ARE DROPPED!!!!!!!");
        return false;
      }

    }


    // Get particle information
    const G4int PDGcode = track.GetDefinition()->GetPDGEncoding();




    const G4ThreeVector & position = 0.5 * (aStep->GetPostStepPoint()->GetPosition() +
                                            aStep->GetPreStepPoint()->GetPosition())
                                     * 0.1; // to convert to cm


    //creates hit
    EKLMSimHit *hit = new  EKLMSimHit(position,  hitTime, PDGcode,  eDep);

    // insert hit to the hit collection
    m_HitCollection->insert(hit);

    return true;
  }


  void EKLMSensetiveDetector::EndOfEvent(G4HCofThisEvent *)
  {
    B2DEBUG(1, " START DIGITIZATION");

    EKLMDigitizer *digi = new EKLMDigitizer(m_HitCollection);

    digi->getSimHits();
    digi->mergeSimHitsToStripHits();
    digi->saveStripHits();

    //    delete(digi);
  }

} //namespace Belle II

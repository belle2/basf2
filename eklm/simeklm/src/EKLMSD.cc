/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <eklm/simeklm/EKLMSD.h>
#include <eklm/simeklm/EKLMDigitizer.h>
#include <framework/logging/Logger.h>


#include "G4Step.hh"
#include "G4SteppingManager.hh"
#include "G4SDManager.hh"
#include "G4TransportationManager.hh"
#include "G4FieldManager.hh"
#include "G4MagneticField.hh"



namespace Belle2 {


  EKLMSD::EKLMSD(G4String name, G4double thresholdEnergyDeposit, G4double thresholdKineticEnergy):
      G4VSensitiveDetector(name), m_ThresholdEnergyDeposit(thresholdEnergyDeposit),
      m_ThresholdKineticEnergy(thresholdKineticEnergy), m_HitCollection(0),
      m_HCID(-1)
  {

    G4String CollName1 = name + "_Collection";
    collectionName.insert(CollName1);
  }

  void EKLMSD::Initialize(G4HCofThisEvent * HCTE)
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
  G4bool EKLMSD::ProcessHits(G4Step *aStep, G4TouchableHistory *)
  {
    // Get deposited energy
    const G4double eDep = aStep->GetTotalEnergyDeposit();

    // ignore tracks with small energy deposition
    double eDepositionThreshold = 0.0;  // should be accessible via xml
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
      ERROR("EKLMSD: global time is nan");
      return false;

      double hitTimeThreshold = 500;
      if (hitTime > hitTimeThreshold) {
        INFO("EKLMSD:  ALL HITS WITH TIME > 500 ns ARE DROPPED!!!!!!!");
        return false;
      }

    }

    const G4int PDGcode = track.GetDefinition()->GetPDGEncoding();
    const G4ThreeVector & position = 0.5 * (aStep->GetPostStepPoint()->GetPosition() +
                                            aStep->GetPreStepPoint()->GetPosition());


    //creates hit
    EKLMSimHit *hit = new  EKLMSimHit(position,  hitTime, PDGcode,  eDep);

    // insert hit to the hit collection
    m_HitCollection->insert(hit);

    hit->Save("/tmp/q_");
    return true;
  }


  void EKLMSD::EndOfEvent(G4HCofThisEvent *)
  {
    DEBUG(1, " START DIGITALIZATION");

    EKLMDigitizer *digi = new EKLMDigitizer(m_HitCollection);

    digi->getSimHits();
    digi->mergeSimHitsToStripHits();
    digi->saveStripHits();

    // Do not forget to delete digi !!!!!!!!!!!!
  }







} //namespace Belle II

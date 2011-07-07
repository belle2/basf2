/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#include <bklm/simbklm/BKLMSensitiveDetector.h>
#include <bklm/simbklm/BKLMDigitizer.h>
#include <bklm/simbklm/BKLMDigitizerPar.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>

#include "G4Step.hh"
#include "G4SteppingManager.hh"
#include "G4EventManager.hh"
#include "G4SDManager.hh"
#include "G4TransportationManager.hh"
#include "G4Material.hh"
#include "G4Event.hh"

namespace Belle2 {

  BKLMSensitiveDetector::BKLMSensitiveDetector(G4String name):
      SensitiveDetectorBase(name),
      m_HitsCollection(0),
      m_HCID(-1)
  {
    collectionName.insert(name + "_HitsCollection");
    //m_RPCGas = G4Material::GetMaterial( "RPCGas" );
    m_neutronPID = 2112;
    //BKLMDigitizerPar* digPar = BKLMDigitizerPar::Instance();
    //m_hitTimeMax = digPar->hitTimeMax();
    m_hitTimeMax = 32000.0;
  }

  void BKLMSensitiveDetector::Initialize(G4HCofThisEvent* HCTE)
  {
    // Create a new hit collection for BKLM
    m_HitsCollection = new BKLMSimHitsCollection(SensitiveDetectorName, collectionName[0]);

    // Assign a unique ID to the BKLM hits collection
    if (m_HCID < 0) {
      m_HCID = G4SDManager::GetSDMpointer()->GetCollectionID(m_HitsCollection);
    }

    // Attach the BKLM hits collection to the event's HitsCollectionsOfThisEvent
    HCTE->AddHitsCollection(m_HCID, m_HitsCollection);

  }

//-----------------------------------------------------
// Method invoked for every step in sensitive detector
//-----------------------------------------------------
  G4bool BKLMSensitiveDetector::ProcessHits(G4Step* aStep, G4TouchableHistory*)
  {

    // It is not necessary to detect motion from one volume to another (or track death
    // in the RPC gas volume).  Experimentation shows that most tracks pass through the
    // RPC gas volume in one step (although, on occasion, a delta ray will take a couple
    // of short steps entirely within gas).  Therefore, save every step in the gas
    // instead of trying to find entry and exit points and then saving only the midpoint.
    // Do same for scintillators.

    G4double     deltaE     = aStep->GetTotalEnergyDeposit();
    G4StepPoint* preStep    = aStep->GetPreStepPoint();
    G4StepPoint* postStep   = aStep->GetPostStepPoint();
    G4Track*     track      = aStep->GetTrack();
    G4int        primaryPID = track->GetDefinition()->GetPDGEncoding();

    // Record a step for a charged track that deposits some energy.
    // Background study: Record every neutron passage, whether it deposits energy or not.
    if (((deltaE > 0.0) && (postStep->GetCharge() == 0)) ||
        (primaryPID == m_neutronPID)) {
      G4ThreeVector hitPos  = 0.5 * (preStep->GetPosition() + postStep->GetPosition());
      G4double      hitTime = 0.5 * (preStep->GetGlobalTime() + postStep->GetGlobalTime());
      if (isnan(hitTime)) return false;             // sanity check
      if (hitTime > m_hitTimeMax) return false;     // drop very late hits (due to fission)
      G4bool decayed = false;
      if (postStep->GetProcessDefinedStep() != 0) {
        decayed = (postStep->GetProcessDefinedStep()->GetProcessType() == fDecay);
      }
      G4bool inRPCGas = (preStep->GetMaterial() == m_RPCGas);      // true: in gas, false: in scint
      BKLMSimHit *hit = new BKLMSimHit(hitPos, hitTime, deltaE, inRPCGas, decayed,
                                       track->GetTrackID(),
                                       primaryPID, track->GetParentID(),
                                       GetAncestorPID(track), GetFirstPID());
      m_HitsCollection->insert(hit);
      return true;
    }
    return false;
  }

  G4int BKLMSensitiveDetector::GetAncestorPID(G4Track* track)
  {
    if (track->GetParentID() != 0) {
      const G4Event* event = G4EventManager::GetEventManager()->GetConstCurrentEvent();
      const G4int nVtx = event->GetNumberOfPrimaryVertex();
      for (G4int jVtx = 0; jVtx < nVtx; jVtx++) {
        const G4PrimaryVertex* vtx = event->GetPrimaryVertex(jVtx);
        const G4int nPart = vtx->GetNumberOfParticle();
        for (G4int jPart = 0; jPart < nPart; jPart++) {
          const G4PrimaryParticle* part = vtx->GetPrimary(jPart);
          if (part->GetTrackID() == track->GetParentID()) {
            return part->GetPDGcode();
          }
        }
      }
    }
    return 0;
  }

  G4int BKLMSensitiveDetector::GetFirstPID()
  {
    // FirstPID should be saved by UserTrackingAction for original generator particle, but isn't yet :(
    //return G4EventManager::GetEventManager()->GetUserTrackingAction()->GetFirstPID();
    return 0;
  }

  void BKLMSensitiveDetector::EndOfEvent(G4HCofThisEvent*)
  {

    B2INFO("BKLMSensitiveDetector::EndOfEvent called");
    //This is duplicate?  is digitizer called automatically by geant4 at end of event?
    //G4DigiManager* digiMgr = G4DigiManager::GetDMpointer();
    //BKLMDigitizer* digitizer = (BKLMDigitizer*)digiMgr->FindDigitizerModule( "BKLMDigitizer" );
    //digitizer->Digitize();

  }

} // end of namespace Belle2

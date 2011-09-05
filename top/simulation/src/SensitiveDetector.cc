/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Petric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <top/simulation/SensitiveDetector.h>
#include <framework/logging/Logger.h>
#include <top/dataobjects/TOPSimHit.h>

#include <G4Track.hh>
#include <G4Step.hh>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <TVector3.h>

using namespace std;

namespace Belle2 {
  namespace top {

    SensitiveDetector::SensitiveDetector():
        Simulation::SensitiveDetectorBase("TOP", SensitiveDetector::TOP)
    {
      StoreArray<MCParticle> mcParticles;
      StoreArray<TOPSimHit>  topSimHits;
      //  RelationArray  topSimHitRel(mcParticles, topSimHits);
      //  registerMCParticleRelation(topSimHitRel);
    }


    G4bool SensitiveDetector::step(G4Step* aStep, G4TouchableHistory*)
    {
      //Get particle ID
      G4Track& track  = *aStep->GetTrack();
      if (track.GetDefinition()->GetParticleName() != "opticalphoton") return false;

      //Get time (check for proper global time)
      const G4double globalTime = track.GetGlobalTime();
      if (isnan(globalTime)) {
        B2ERROR("TOP Sensitive Detector: global time is nan !");
        return false;
      }

      //Get step information
      const G4StepPoint& preStep  = *aStep->GetPreStepPoint();

      //Get world position
      const G4ThreeVector& worldPosition = preStep.GetPosition();

      //Transform to local position
      const G4ThreeVector localPosition = preStep.GetTouchableHandle()->GetHistory()->GetTopTransform().TransformPoint(worldPosition);

      //Get module ID number
      const G4int moduleID = preStep.GetTouchableHandle()->GetReplicaNumber(1);
      const G4int barID = preStep.GetTouchableHandle()->GetReplicaNumber(3);

      //! B2INFO("replica number: " << moduleID << " bar number: " << barID);

      //Get photon energy
      const G4double energy = track.GetKineticEnergy() / eV;

      //Get ID of parent particle
      const G4int parentID = track.GetParentID();

      //------------------------------------------------------------
      //                Create TOPSimHit and save it to datastore
      //------------------------------------------------------------

      TVector3 locpos(localPosition.x() / cm, localPosition.y() / cm, localPosition.z() / cm);
      StoreArray<TOPSimHit> topSimHits;
      G4int nentr = topSimHits->GetEntries();
      new(topSimHits->AddrAt(nentr)) TOPSimHit(moduleID, barID, locpos, globalTime, energy, parentID);

      // after detection photon track is killed */
      track.SetTrackStatus(fStopAndKill);

      return true;
    }


  } // end of namespace top
} // end of namespace Belle2

/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <arich/simulation/SensitiveDetector.h>
#include <framework/logging/Logger.h>
#include <arich/dataobjects/ARICHSimHit.h>

#include <G4Track.hh>
#include <G4Step.hh>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <TVector3.h>

using namespace std;

namespace Belle2 {
  namespace arich {

    SensitiveDetector::SensitiveDetector():
        Simulation::SensitiveDetectorBase("ARICH", SensitiveDetector::TOP)
    {
      StoreArray<MCParticle> mcParticles;
      StoreArray<ARICHSimHit>  arichSimHits;

      RelationArray  arichSimHitRel(mcParticles, arichSimHits);
      registerMCParticleRelation(arichSimHitRel);
    }


    G4bool SensitiveDetector::step(G4Step* aStep, G4TouchableHistory*)
    {
      //Get particle ID
      G4Track& track  = *aStep->GetTrack();
      // if (track.GetDefinition()->GetParticleName() != "opticalphoton") return false;

      //Get time (check for proper global time)
      const G4double globalTime = track.GetGlobalTime();
      if (isnan(globalTime)) {
        B2ERROR("ARICH Sensitive Detector: global time is nan !");
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
      B2INFO("replica number: " << moduleID);
      //Get photon energy
      const G4double energy = track.GetKineticEnergy() / eV;

      //Get ID of parent particle
      const G4int parentID = track.GetParentID();

      //------------------------------------------------------------
      //                Create ARICHSimHit and save it to datastore
      //------------------------------------------------------------

      TVector3 locpos(localPosition.x() / cm, localPosition.y() / cm, localPosition.z() / cm);
      StoreArray<ARICHSimHit> arichSimHits;
      int nentr = arichSimHits->GetLast() + 1;
      new(arichSimHits->AddrAt(nentr)) ARICHSimHit(moduleID, locpos, globalTime, energy, parentID);

      // after detection photon track is killed
      track.SetTrackStatus(fStopAndKill);

      return true;
    }


  } // end of namespace arich
} // end of namespace Belle2

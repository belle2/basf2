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
#include <framework/datastore/RelationIndex.h>
#include <TVector3.h>

using namespace std;

namespace Belle2 {
  namespace arich {

    SensitiveDetector::SensitiveDetector():
      Simulation::SensitiveDetectorBase("ARICH", Const::ARICH)
    {

      StoreArray<ARICHSimHit>::registerPersistent();
      RelationArray::registerPersistent<MCParticle, ARICHSimHit>();

      StoreArray<MCParticle> particles;
      StoreArray<ARICHSimHit> hits;
      RelationArray relation(particles, hits);
      registerMCParticleRelation(relation);

    }


    G4bool SensitiveDetector::step(G4Step* aStep, G4TouchableHistory*)
    {
      //Get particle ID
      G4Track& track  = *aStep->GetTrack();
      if (track.GetDefinition()->GetParticleName() != "opticalphoton") return false;

      //Get time (check for proper global time)
      const G4double globalTime = track.GetGlobalTime();
      if (std::isnan(globalTime)) {
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
      //Get photon energy
      const G4double energy = track.GetKineticEnergy() / eV;

      //Get ID of parent particle
      const G4int parentID = track.GetParentID();
      //Get id of particle
      const G4int trackID = track.GetTrackID();

      //------------------------------------------------------------
      //                Create ARICHSimHit and save it to datastore
      //------------------------------------------------------------

      TVector3 locpos(localPosition.x() / cm, localPosition.y() / cm, localPosition.z() / cm);
      StoreArray<ARICHSimHit> arichSimHits;
      if (!arichSimHits.isValid()) arichSimHits.create();
      new(arichSimHits.nextFreeAddress()) ARICHSimHit(moduleID, locpos, globalTime, energy, parentID);

      // add relation to MCParticle
      StoreArray<MCParticle> mcParticles;
      RelationArray  arichSimHitRel(mcParticles, arichSimHits);
      int nentr = arichSimHits.getEntries() - 1;
      arichSimHitRel.add(trackID, nentr);

      // after detection photon track is killed
      track.SetTrackStatus(fStopAndKill);

      return true;
    }


  } // end of namespace arich
} // end of namespace Belle2

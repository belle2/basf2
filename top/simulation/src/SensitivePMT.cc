/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Petric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <top/simulation/SensitivePMT.h>

#include <top/dataobjects/TOPSimHit.h>

#include <G4Track.hh>
#include <G4Step.hh>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/logging/Logger.h>
#include <TVector3.h>

using namespace std;

namespace Belle2 {
  namespace TOP {

    SensitivePMT::SensitivePMT():
      Simulation::SensitiveDetectorBase("TOP", SensitivePMT::TOP)
    {
      StoreArray<MCParticle> mcParticles;
      StoreArray<TOPSimHit>  topSimHits;

      RelationArray  relTOPSimHitToMCParticle(topSimHits, mcParticles);
      registerMCParticleRelation(relTOPSimHitToMCParticle);
    }


    G4bool SensitivePMT::step(G4Step* aStep, G4TouchableHistory*)
    {
      StoreArray<MCParticle> mcParticles;

      //Get particle ID
      G4Track& track  = *aStep->GetTrack();
      //Get step information
      const G4StepPoint& preStep  = *aStep->GetPreStepPoint();

      if (track.GetDefinition()->GetParticleName() != "opticalphoton") return false;

      //B2INFO(track.GetDefinition()->GetPDGEncoding());

      //Get time (check for proper global time)
      const G4double globalTime = track.GetGlobalTime();
      const G4double localTime = track.GetLocalTime();
      if (isnan(globalTime)) {
        B2ERROR("TOP Sensitive Detector: global time is nan !");
        return false;
      }



      //Get world position
      const G4ThreeVector& worldPosition = track.GetPosition();

      if (fabs(worldPosition.z() + 800.5) > 10e-5) return false;


      //Transform to local position
      const G4ThreeVector localPosition = track.GetTouchableHandle()->GetHistory()->GetTopTransform().TransformPoint(worldPosition);

      //Get module ID number
      const G4int moduleID = track.GetTouchableHandle()->GetReplicaNumber(1);
      const G4int barID = track.GetTouchableHandle()->GetReplicaNumber(1);

      //! B2INFO("replica number: " << moduleID << " bar number: " << barID);

      //Get photon energy
      const G4double energy = track.GetKineticEnergy();

      const G4double length = track.GetTrackLength();

      //Get ID of parent particle and track
      const G4int parentID = track.GetParentID();
      const G4int trackID = track.GetTrackID();

      const G4ThreeVector vpos = track.GetVertexPosition();
      const G4ThreeVector vdir = track.GetVertexMomentumDirection();
      const G4ThreeVector dir = track.GetMomentumDirection();

      //------------------------------------------------------------
      //                Create TOPSimHit and save it to datastore
      //------------------------------------------------------------

      TVector3 locpos(localPosition.x(), localPosition.y() , localPosition.z());
      TVector3 glopos(worldPosition.x() , worldPosition.y() , worldPosition.z());
      TVector3 Vpos(vpos.x() , vpos.y() , vpos.z());
      TVector3 Vdir(vdir.x() , vdir.y() , vdir.z());
      TVector3 Dir(dir.x() , dir.y() , dir.z());


      StoreArray<TOPSimHit> topSimHits;
      G4int nentr = topSimHits->GetEntries();
      new(topSimHits->AddrAt(nentr)) TOPSimHit(moduleID, barID, locpos, glopos, Dir, Vpos, Vdir, globalTime, globalTime - localTime, length, energy, parentID, trackID);

      RelationArray relTOPSimHitToMCParticle(topSimHits, mcParticles);
      relTOPSimHitToMCParticle.add(nentr, trackID);


      // after detection photon track is killed */
      track.SetTrackStatus(fStopAndKill);

      return true;
    }


  } // end of namespace top
} // end of namespace Belle2

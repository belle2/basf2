/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Petric, Marko Staric                                             *
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
#include <framework/gearbox/Unit.h>
#include <TVector3.h>

using namespace std;

namespace Belle2 {
  namespace TOP {

    SensitivePMT::SensitivePMT():
      Simulation::SensitiveDetectorBase("TOP", SensitivePMT::TOP)
    {
      // registration
      StoreArray<TOPSimHit>::registerPersistent();
      RelationArray::registerPersistent<MCParticle, TOPSimHit>();

      // additional registration of MCParticle relation (required for correct relations)
      StoreArray<MCParticle> particles;
      StoreArray<TOPSimHit>  hits;
      RelationArray  relation(particles, hits);
      registerMCParticleRelation(relation);
    }


    G4bool SensitivePMT::step(G4Step* aStep, G4TouchableHistory*)
    {
      //! Get particle track
      G4Track& track  = *aStep->GetTrack();

      /*! Check if it is an optical photon */
      if (track.GetDefinition()->GetParticleName() != "opticalphoton") return false;

      /*! Get time (check for proper global time) of track */

      //! get global time
      double globalTime = track.GetGlobalTime();
      //! get local time. Time that passed from the cration of the particle.
      double localTime = track.GetLocalTime();

      //! Check all the times exist. Maybe this is not necesary!
      if (isnan(globalTime)) {
        B2ERROR("TOP Sensitive PMT: global time is nan !");
        return false;
      }
      if (isnan(localTime)) {
        B2ERROR("TOP Sensitive PMT: local time is nan !");
        return false;
      }


      //! get the possition of the particle in the lab frame - global possition
      const G4ThreeVector& worldPosition = track.GetPosition();

      //! Transform to local position
      const G4ThreeVector localPosition = track.GetTouchableHandle()->GetHistory()->GetTopTransform().TransformPoint(worldPosition);


      //! get direction of the momentum at hit point
      const G4ThreeVector dir = track.GetMomentumDirection();

      //! get pmt ID
      int pmtID = track.GetTouchableHandle()->GetReplicaNumber(1);
      //! get ID of the bar in which the pmt is housed
      int barID = track.GetTouchableHandle()->GetReplicaNumber(4);

      //! This is here just for debuging
      //B2INFO("replica number: " << pmtID << " bar number: " << barID)

      //! get photon energy
      double energy = track.GetKineticEnergy();

      //! get the length of the track
      double length = track.GetTrackLength();

      //Get ID of parent particle and track (this is mainly used at the moment for separation of backgound and signal, but will be later dropped -> use relation )
      int parentID = track.GetParentID();
      int trackID = track.GetTrackID();

      /*! get vertex information of the track */
      //! get vertex position
      const G4ThreeVector vpos = track.GetVertexPosition();
      //! get direction of the momentun at the vertex point
      const G4ThreeVector vdir = track.GetVertexMomentumDirection();

      /*! Fill vector for later storage in TOPSimHit */

      //! fill vector for local position
      TVector3 locpos(localPosition.x(), localPosition.y() , localPosition.z());
      //! fill vector for global position
      TVector3 glopos(worldPosition.x() , worldPosition.y() , worldPosition.z());
      //! fill vector for vertex position
      TVector3 Vpos(vpos.x() , vpos.y() , vpos.z());
      //! fill vector for momentum direction at the vertex
      TVector3 Vdir(vdir.x() , vdir.y() , vdir.z());
      //! fill vector for momentum direction at hit point
      TVector3 Dir(dir.x() , dir.y() , dir.z());

      //! convert to Basf units (photon energy in [eV]!)
      locpos = locpos * Unit::mm;
      glopos = glopos * Unit::mm;
      Vpos = Vpos * Unit::mm;
      length = length * Unit::mm;
      energy = energy * Unit::MeV / Unit::eV;

      double x = locpos.x();
      double y = locpos.y();

      /*!------------------------------------------------------------
       *                Create TOPSimHit and save it to datastore
       * ------------------------------------------------------------
       */

      StoreArray<TOPSimHit> topSimHits;
      if (!topSimHits.isValid()) topSimHits.create();

      new(topSimHits.nextFreeAddress()) TOPSimHit(barID, pmtID, x, y, globalTime, energy);

      // add relation to MCParticle
      StoreArray<MCParticle> mcParticles;
      RelationArray relMCParticleToTOPSimHit(mcParticles, topSimHits);
      int last = topSimHits.getEntries() - 1;
      relMCParticleToTOPSimHit.add(trackID, last);


      /*! After detection photon is killed */

      track.SetTrackStatus(fStopAndKill);

      return true;
    }


  } // end of namespace top
} // end of namespace Belle2
